#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<time.h>
#include<arduPi.h>

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
	int rfid;
	};
FILE *logfile;

// initialisiere Logfile
int init_log() {
	logfile=fopen("/home/uwe/Raspberry/sensor/log/senso.log","a");
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
	fprintf(logfile,"%s: %s\n",timestr,str);
	}

/* Funktionsdeklaration */
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
	ad->rfid=815;
	return 0;
	}

/* Hauptprogramm */

int main() {
	int ret;
	int ch;
	char logstr[200];
	struct addr ad;
	
	init_log();
	
	log("Messdatenerfassung");
	log("Ablauf:");
	log("\t-GPS Standorterfassung");
	log("\t-Erfassung Nummer Messglass");
	log("\t-warte auf Taste und lese dann Temperatursensor");
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
	ret=get_rfid (&ad);
	if ( ret != 0 ) {
		log("Fehler beim auslesen der RFID-Daten!!");
		return 1;
	} else {
		log("RFID erfolgreich ausgelesen:");
		sprintf(logstr,"RFID Nummer: %d",ad.rfid);
		log(logstr);
	}
	log("warte auf Taste.....");
	ch=getchar();
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

	return 0;
	}
