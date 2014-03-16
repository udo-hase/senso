//Programm zur Abwasser-Messdatenerfassung 
//  - auslesen GPS Position
//  - auslesen RFID Sensor vom Messglass
//  - auslesen Probetemperatur
//

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <arduPi.h>



#define DEBUG 1
#define LCD "../bin/lcd_senso"

#define LOGFILE "/home/pi/senso/log/senso.log"
#define DATAFILE "/home/pi/senso/data/data.csv"

//define Adresse vom temp Sensor am W1 Bus
#define SENSOR "/sys/bus/w1/devices/28-000004d08e2d/w1_slave" //jeder Slave am W1 BUS hat eine eigene ID!!!

/*Variablendefinition */
struct addr {
	char vendor[20];
	char lat[15];
	char n;
	char lon[15];
	char w;
	char utc[15];
	char gps_valid;
	char mode;
	char checksum[10];
	char temp[10];
	char temp_valid[10];
	char rfid[10];
	};
	 
FILE *logfile; //LOGFILE
FILE *dfile;   //Datenfile

//define substring function
char *substr(size_t start, size_t stop, const char *src, char *dst, size_t size)
{
   int count = stop - start;
   if ( (unsigned)count >= --size )
   {
      count = size;
   }
   sprintf(dst, "%.*s", count, src + start);
   return dst;
}

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
	fprintf(dfile,"%s;%c;%s;%c;%s,%s;%s\n",ad->lat,ad->n,ad->lon,ad->w,ad->utc,ad->rfid,ad->temp);
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



//gps sensor initialisierung
void setup_gps(){
  //setup for mySerial port
  Serial.begin(4800);  
  printf("set serial speed to 4800..\n");
  delay(1000);
}


//rfid sensor initialisierung
void setup_rfid(){
	int led = 13;
        // Start serial port 19200 bps
        Serial.begin(19200);
        printf("set serial speed to 19200..\n");
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
// variables
byte byteGPS = 0;
int i = 0;
int h = 0;

// Buffer for data input
char GPS_GGA[100]="";
char GPS_BUFFER[100]="";

//Ausgabe des GPS Sensors
//$GPGGA,152145.000,4805.8193,N,01132.2317,E,1,04,2.5,607.5,M,47.6,M,,*67
// Read GGA sentence from GPS
  ad->gps_valid=0; //invalidate GPS Data per see
  do {
	byteGPS = 0;
	byteGPS = Serial.read();
	while(byteGPS != 'A'){
		byteGPS = Serial.read();
	}
	GPS_GGA[0]='$';
	GPS_GGA[1]='G';
	GPS_GGA[2]='P';    
	GPS_GGA[3]='G';
	GPS_GGA[4]='G';
	GPS_GGA[5]='A';
  
    i = 6;
    while(byteGPS != '*'){                  
      byteGPS = Serial.read();         
      GPS_GGA[i]=byteGPS;
      i++;                      
	}
	GPS_GGA[++i]=0x00;  
	// print the GGA sentence to Log
	printf("GGA sentence: ");
	h = 0;
	while(GPS_GGA[h] != 42){
		printf("%c",GPS_GGA[h]);
		h++;
	}
	printf("\n");
  
	strcpy(ad->vendor,"$GPGGA");//Vendor and message identifier
	strcpy(ad->lat,substr(18,28,GPS_GGA,GPS_BUFFER,10));//Latitude (37deg 23.2475min)
	ad->n=GPS_GGA[28];//N North; S South
	strcpy(ad->lon,substr(30,40,GPS_GGA,GPS_BUFFER,sizeof GPS_BUFFER));//Longitude (121deg 58.3416min)
	ad->w=GPS_GGA[41];//W West; E East
	strcpy(ad->utc,substr(7,17,GPS_GGA,GPS_BUFFER,sizeof GPS_BUFFER));//UTC - Universal Time Coordinated (16h 12m 29.487s)
	ad->gps_valid=GPS_GGA[43];//1 Data Valid; 0 Data not Valid
  } while (ad->gps_valid != '1');	
  return 0;
}

/*Funktion liest den Temepratur-Sensor aus*/
int get_temp(struct addr *ad) {
	FILE *w1_slave;
	char tmp[100];
	char yes[2][10];
	char temp[2][10];
	int i=0;
	int h=0; //helper var for determining that two following values are the same
	int same=0; //helper var, to set if two following values the same
	yes[0][0]=0x00;yes[1][0]=0x00;
	temp[0][0]=0x00;temp[1][0]=0x00;
	
	same=0;h=0;
	while(same == 0) {
		if( (w1_slave=fopen(SENSOR,"r")) == NULL){
			printf("could not open Sesnor Device %s\n",SENSOR);
		}
		//read 5 lines of rubish
		for(i=0;i<4;i++){
			fgets(tmp,10,w1_slave);
		}
		//read the YES Flage, if the temp will be valid
		fgets(yes[h],10,w1_slave); yes[h][3]=0x00;
		//read 4 lines of rubish
		for(i=0;i<3;i++){
			fgets(tmp,10,w1_slave);
		}
		//read the temp
		fgets(temp[h],10,w1_slave);temp[h][7]=0x00;
		printf("yes[0]-->%s<--\n",yes[0]);
		printf("temp[0]-->%s<--\n",&temp[0][2]);
		printf("yes[1]-->%s<--\n",yes[1]);
		printf("temp[1]-->%s<--\n",&temp[1][2]);
		printf("------------------------\n");
		if(strcmp(temp[0],temp[1]) == 0 && strcmp(yes[h],"YES") == 0){
			same=1;	
		} else {
			h=1-h;
			sleep(5);
		}
		fclose(w1_slave);
	}
	//printf("yes[%d]-->%s<--\n",h,yes[h]);
	//printf("temp[%d]-->%s<--\n",h,temp[h]);
	sprintf(ad->temp,"%s",&temp[h][2]);
	sprintf(ad->temp_valid,"%s",yes[h]);
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

		
	log("########################################\n");
	log("Messdatenerfassung ");
	sprintf(logstr,"Bibliotheksrevision: %d",REV);
	log(logstr);
	log("initialisiere LCD");
	//system("../bin/lcd_senso i");
	
	log("lese GPS-Standortdaten");
	//system("../bin/lcd_senso p 0 0 103 'lese GPS Daten  bitte warten...'");
	sleep(5);
	log("initialisiere gps sensor..");
	//setup_gps();
	//ret=get_gps (&ad);
	ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der GPS-Daten!!");
		return 1;
	} else {
		log("GPS erfolgreich ausgelesen:");
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'GPS Data gelesen Bitte Select-->'");
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
		sprintf(logstr,"\t-valid Data:\t%c",ad.gps_valid);
		log(logstr);
		
	}
	//system("../bin/lcd_senso w");
	log("lese RFID Sensor aus");
	log("initialisiere rfid sensor..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese RFID Daten bitte warten...'");
	//setup_rfid();
	//ret=get_rfid (&ad);
	//sleep(5);
	ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der RFID-Daten!!");
		return 1;
	} else {
		log("RFID erfolgreich ausgelesen:");
		sprintf(logstr,"RFID Nummer: %s",ad.rfid);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'RFID gelesen    Bitte Select-->'");
	}
	log("warte auf Taste.....");
	getchar();
	//system("../bin/lcd_senso w");
	log("lese Temperatursensor aus..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese Probetemp. bitte warten...'");
	//sleep(5);
	ret=get_temp (&ad);
	if ( ret != 0 ) {
		log("Fehler beim auslesen der Temperatur!!");
		return 1;
	} else {
		log("Temperatur erfolgreich ausgelesen:");
		sprintf(logstr,"Temperatur: %s°C",ad.temp);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'Temp gelesen    Bitte Select-->'");
	}
	//write data to file
	ret=write_data(&ad);
	if ( ret != 0 ) {
		log("Fehler beim schreiben des Datensatzes!");
		return 1;
	} else {
		log("Datensatz erfolgreich geschrieben");
	}
	log("#############################################\n");
	fclose(dfile);
	fclose(logfile);
	return 0;
	}
