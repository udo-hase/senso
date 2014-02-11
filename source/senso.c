//Programm zur Abwasser-Messdatenerfassung 
//  - auslesen GPS Position
//  - auslesen RFID Sensor vom Messglass
//  - auslesen Probetemperatur
//

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<time.h>
#include<arduPi.h>

#define DEBUG 1
#define LOGFILE "/home/pi/senso/log/senso.log"
#define DATAFILE "/home/pi/senso/data/data.csv"

/*Variablendefinition */
struct addr {
	char vendor[20];
	char lat[15];
	char n;
	char lon[15];
	char w;
	char utc[15];
	char valid;
	char mode;
	char checksum[10];
	int temp;
	char rfid[10];
	};
FILE *logfile; //LOGFILE
FILE *dfile;   //Datenfile

// initialisiere Logfile
int init_log() {
	if (DEBUG) {
		printf("using Logfile %s....\n",LOGFILE);
	}
	if( (logfile=fopen(LOGFILE,"a")) == NULL){
		return(-1);
	}
	return(0);
}

//initialisieren Datafile
int init_data() {
	if (DEBUG) {
		printf("using Datafile %s....\n",DATAFILE);
	}
	if( (dfile=fopen(DATAFILE,"a")) == NULL){
		return(-1);
	}
	return(0);
}

int write_data( struct addr *ad ){
	fprintf(dfile,"%s;%c;%s;%c;%s,%s;%d\n",ad->lat,ad->n,ad->lon,ad->w,ad->utc,ad->rfid,ad->temp);
	return (0);	
}

// log output
void log(const char *str) {
	time_t mytime;
	char timestr[50];
	int len;
	mytime=time(NULL);
	sprintf(timestr,"%s",ctime(&mytime));
	len=strlen(timestr);
	timestr[len-1]=0x00;
	if (DEBUG) {
		printf("%s: %s\n",timestr,str);
	}
	fprintf(logfile,"%s: %s\n",timestr,str);
}

//rfid sensor initialisierung
void setup_rfid(){
	int led = 13;
        // Start serial port 19200 bps
        Serial.begin(19200);
        pinMode(led, OUTPUT);

        delay(500);

        // Setting Auto Read Mode - EM4102 Decoded Mode - No password
        // command: FF 01 09 87 01 03 02 00 10 20 30 40 37
        Serial.print(0xFF,BYTE);
        Serial.print(0x01,BYTE);
        Serial.print(0x09,BYTE);
        Serial.print(0x87,BYTE);
        Serial.print(0x01,BYTE);
        Serial.print(0x03,BYTE);
        Serial.print(0x02,BYTE);
        Serial.print(0x00,BYTE);
        Serial.print(0x10,BYTE);
        Serial.print(0x20,BYTE);
        Serial.print(0x30,BYTE);
        Serial.print(0x40,BYTE);
        Serial.print(0x37,BYTE);

        delay(500);
        Serial.flush();
        log("RFID module started in Auto Read Mode\n");
}



/*Funktion liest den GPS-Sensor aus*/
int get_gps( struct addr *ad) {
//Ausgabe des GPS Sensors
//$GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41
	strcpy(ad->vendor,"$GPGSV");//Vendor and message identifier
	strcpy(ad->lat,"3723.2475");//Latitude (37deg 23.2475min)
	ad->n='N';//N North; S South
	strcpy(ad->lon,"12158.3416");//Longitude (121deg 58.3416min)
	ad->w='W';//W West; E East
	strcpy(ad->utc,"161229.487");//UTC - Universal Time Coordinated (16h 12m 29.487s)
	ad->valid='A';//A Data Valid; V Data not Valid
	ad->mode='A';//A Autonomous mode; D DGPS mode; E DR mode 
	strcpy(ad->checksum,"*41");//Checksum
	if (ad->valid != 'A') {
			
	}
	return 0;
	}

/*Funktion liest den Temepratur-Sensor aus*/
int get_temp(struct addr *ad) {
	ad->temp=20;
	return 0;
	}

/*Funktion liest den RFID-Sensor aus*/
int get_rfid(struct addr *ad) {
	int led = 13;
	byte data_1 = 0x00;
	byte data_2 = 0x00;
	byte data_3 = 0x00;
	byte data_4 = 0x00;
	byte data_5 = 0x00;
	int val = 0;

	val = Serial.read();
    while (val != 0xff){
        log("Waiting card\n");
        val = Serial.read();
        delay(1000);
    }

    // Serial.read();    // we read ff
    Serial.read();    // we read 01
    Serial.read();    // we read 06
    Serial.read();    // we read 10
    data_1 = Serial.read();    // we read data 1
    data_2 = Serial.read();    // we read data 2
    data_3 = Serial.read();    // we read data 3
    data_4 = Serial.read();    // we read data 4
    data_5 = Serial.read();    // we read data 5
    Serial.read();    // we read checksum


    // Led blink
    for(int i = 0;i < 4;i++){
       digitalWrite(led,HIGH);
       delay(500);
       digitalWrite(led,LOW);
       delay(500);
    }

    // Printing the code of the card
    log("EM4100 card found - Code: ");
    sprintf(ad->rfid,"%x%x%x%x%x",data_1,data_2,data_3,data_4,data_5);
	return 0;
	}

/* Hauptprogramm */

int main() {
	int ret;
	char logstr[200];
	struct addr ad;
	
	if(init_log() != 0 ) {
		printf("Fehler beim initialisieren des Logfile! \n");
		return (-1);
	};
	
	if(init_data() != 0 ) {
		printf("Fehler beim initialisieren des Datenfile! \n");
		return (-1);
	};
	
	log("Messdatenerfassung ");
	sprintf(logstr,"Bibliotheksrevision: %d",REV);
	log(logstr);
	log("----------------------------------------------------");
	
	log("lese GPS-Standortdaten");
	ret=get_gps (&ad);
	if ( ret != 0 ) {
		log("Fehler beim auslesen der GPS-Daten!!");
		return 1;
	} else {
		log("GPS erfolgreich ausgelesen:");
		log("GPS Standort:");
		sprintf(logstr,"\t-Vendor:\t%s",ad.vendor);
		log(logstr);
		sprintf(logstr,"\t-Latitude:\t%s",ad.lat);
		log(logstr);
		sprintf(logstr,"\t-North:\t\t%c",ad.n);
		log(logstr);
		sprintf(logstr,"\t-Longitude:\t%s",ad.lon);
		log(logstr);
		sprintf(logstr,"\t-West:\t\t%c",ad.w);
		log(logstr);
		sprintf(logstr,"\t-UTC:\t\t%s",ad.utc);
		log(logstr);
		sprintf(logstr,"\t-valid Data:\t%c",ad.valid);
		log(logstr);

	}
	log("lese RFID Sensor aus");
	log("initialisiere rfid sensor..");
	setup_rfid();
	ret=get_rfid (&ad);
	if ( ret != 0 ) {
		log("Fehler beim auslesen der RFID-Daten!!");
		return 1;
	} else {
		log("RFID erfolgreich ausgelesen:");
		sprintf(logstr,"RFID Nummer: %s",ad.rfid);
		log(logstr);
	}
	log("warte auf Taste.....");
	getchar();
	log("lese Temperatursensor aus..");
	ret=get_temp (&ad);
	if ( ret != 0 ) {
		log("Fehler beim auslesen der Temperatur!!");
		return 1;
	} else {
		log("Temperatur erfolgreich ausgelesen:");
		sprintf(logstr,"Temperatur: %d°C",ad.temp);
		log(logstr);
	}
	//write data to file
	ret=write_data(&ad);
	if ( ret != 0 ) {
		log("Fehler beim schreiben des Datensatzes!");
		return 1;
	} else {
		log("Datensatz erfolgreich geschrieben");
	}
	fclose(dfile);
	fclose(logfile);
	return 0;
	}
