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
#include <stdlib.h>
#include <syslog.h>
#include <sys/wait.h>
#include <signal.h>
#include <wiringSerial.h>
#include <mcp23017.h>
#include <lcd.h>
#include "rfid.h"
#include "bcm2835.h"
#include "config.h"
#include "main.h"

//#include <arduPi.h>


#define REV 01

#define LCD "../bin/lcd_senso"

#define LOGFILE "/home/pi/senso/log/senso.log"
#define DATAFILE "/home/pi/senso/data/data.csv"

//define Adresse vom temp Sensor am W1 Bus
#define SENSOR "/sys/bus/w1/devices/28-000004d08e2d/w1_slave" //jeder Slave am W1 BUS hat eine eigene ID!!!


uint8_t HW_init(uint32_t spi_speed, uint8_t gpio); //rfid init

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
	char k_temp[10];
	char k_temp_valid[10];
	char k_rfid[10];
	char w_temp[10];
	char w_temp_valid[10];
	char w_rfid[10];
	};
	 
FILE *logfile; //LOGFILE
FILE *dfile;   //Datenfile
int serial_fd; //serial port FD


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
	fprintf(dfile,"%s;%c;%s;%c;%s,%s;%s;%s;%s\n",ad->lat,ad->n,ad->lon,ad->w,ad->utc,ad->k_rfid,ad->k_temp,ad->w_rfid,ad->w_temp);
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
  
  //Serial.begin(4800);  
  serial_fd=serialOpen("/dev/ttyAMA0",4800);
  if ( serial_fd == -1) {
	  printf("kann GPS Device AMA0 nicht öffnen...\n");
  } else { 
	printf("set serial speed for /dev/ttyAMA0 to 4800..\n");
  }
  delay(1000);
}


//rfid sensor initialisierung
uint8_t HW_init(uint32_t spi_speed, uint8_t gpio) {
	uint16_t sp;

	sp=(uint16_t)(250000L/spi_speed);
	if (!bcm2835_init()) {
		syslog(LOG_DAEMON|LOG_ERR,"Can't init bcm2835!\n");
		return 1;
	}
	if (gpio<28) {
		bcm2835_gpio_fsel(gpio, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_write(gpio, LOW);
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
	bcm2835_spi_setClockDivider(sp); // The default
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
	return 0;
}
/* rfid sensor mit ardulib von cooking hacks
 * 
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
*/


/*Funktion liest den GPS-Sensor aus*/

int get_gps( struct addr *ad) {
// variables
char byteGPS = ' ';
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
	byteGPS = serialGetchar(serial_fd);
	while(byteGPS != 'A'){
		byteGPS = serialGetchar(serial_fd);
	}
	GPS_GGA[0]='$';
	GPS_GGA[1]='G';
	GPS_GGA[2]='P';    
	GPS_GGA[3]='G';
	GPS_GGA[4]='G';
	GPS_GGA[5]='A';
  
    i = 6;
    while(byteGPS != '*'){                  
      byteGPS = serialGetchar(serial_fd);         
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


/*Funktion liest den Temperatur-Sensor aus*/
int get_temp(struct addr *ad, char kw) {
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
	switch (kw) {
		case 'k' : sprintf(ad->k_temp,"%s",&temp[h][2]);
					sprintf(ad->k_temp_valid,"%s",yes[h]);
					break;
		case 'w' : sprintf(ad->w_temp,"%s",&temp[h][2]);
					sprintf(ad->w_temp_valid,"%s",yes[h]);
					break;
	}
	return 0;
}

/*Funktion liest den RFID-Sensor aus*/
int get_rfid(struct addr *ad, char kw) {

	uid_t uid;
	uint8_t SN[10];
	uint16_t CType=0;
	uint8_t SN_len=0;
	char status;
	int tmp,i;

	char str[255];
	char *p;
	char sn_str[23];
	pid_t child;
	int max_page=0;
	uint8_t page_step=0;

	FILE * fmem_str;
	char save_mem=0;
	char fmem_path[255];
	uint8_t use_gpio=0;
	uint8_t gpio=255;
	uint32_t spi_speed=10000000L;

	if (open_config_file(config_file)!=0) {
		if (DEBUG) {fprintf(stderr,"Can't open config file!");}
		else{syslog(LOG_DAEMON|LOG_ERR,"Can't open config file!");}
		return 1;
	}
	if (find_config_param("GPIO=",str,sizeof(str)-1,1)==1) {
		gpio=(uint8_t)strtol(str,NULL,10);
		if (gpio<28) {
			use_gpio=1;
		} else {
			gpio=255;
			use_gpio=0;
		}
	}
	if (find_config_param("SPI_SPEED=",str,sizeof(str)-1,1)==1) {
		spi_speed=(uint32_t)strtoul(str,NULL,10);
		if (spi_speed>125000L) spi_speed=125000L;
		if (spi_speed<4) spi_speed=4;
	}

	if (HW_init(spi_speed,gpio)) return 1; // Если не удалось инициализировать RC522 выходим.
	if (read_conf_uid(&uid)!=0) return 1;
	setuid(uid);
	InitRc522();
	
	for (;;) {
		status= find_tag(&CType);
		//printf("(TAG Status: %c\n",status);
		if (status==TAG_NOTAG) {
			usleep(200000);
			continue;
		}else if ((status!=TAG_OK)&&(status!=TAG_COLLISION)) {continue;}

		if (select_tag_sn(SN,&SN_len)!=TAG_OK) {continue;}

		p=sn_str;
		//*(p++)='[';
		for (tmp=0;tmp<SN_len;tmp++) {
			sprintf(p,"%02x",SN[tmp]);
			p+=2;
		}
		//for debugging
		if (DEBUG) {
		*p=0;
		printf("Type: %04x, Serial: %s\n",CType,&sn_str[1]);
		}
		//*(p++)=']';
		*(p++)=0;
		
		if (use_gpio) bcm2835_gpio_write(gpio, HIGH);
		break;
	}
	switch (kw) {
		case 'k' : sprintf(ad->k_rfid,"%s",&sn_str[1]);
					break;
		case 'w' : sprintf(ad->w_rfid,"%s",&sn_str[1]);
					break;
		}
	bcm2835_spi_end();
	bcm2835_close();
	close_config_file();
	return 0;
}
/* rfid sesnor mit ardulib von cooking hacks
 * 
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
*/

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
	//init the LCD
	lcd_init();
	lcd_clear();
	//system("../bin/lcd_senso i");
	log("lese GPS-Standortdaten");
	lcd_print(0,0,103,"lese GPS Daten  bitte warten...");
	//system("../bin/lcd_senso p 0 0 103 'lese GPS Daten  bitte warten...'");
	//sleep(5);
	log("initialisiere gps sensor..");
	setup_gps();
	ret=get_gps (&ad);
	serialClose(serial_fd);
	//ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der GPS-Daten!!");
		lcd_clear();
		lcd_print(0,0,103,"Fehler beim auslesen der GPS-Daten!!");
		return 1;
	} else {
		log("GPS erfolgreich ausgelesen:");
		lcd_clear();
		lcd_print(0,0,103,"GPS Position ermittelt:");
		
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
	// Kaltwasserwerte auslesen...
	lcd_print(0,1,103,"RFID Kaltwasser -> Next");
	waitForEnter();
	//system("../bin/lcd_senso w");
	log("lese RFID Sensor aus");
	log("initialisiere rfid sensor..");
	lcd_clear();
	lcd_print(0,0,103,"RFID wird gesucht..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese RFID Daten bitte warten...'");
	ret=get_rfid (&ad,'k'); //lese Kaltwasser RFID
	//ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der RFID-Daten!!");
		lcd_clear();
		lcd_print(0,0,103,"Fehler bei lesen der RFID-Werte..");
		return 1;
	} else {
		log("RFID erfolgreich ausgelesen:");
		sprintf(logstr,"RFID: %s",ad.k_rfid);
		lcd_clear();
		lcd_print(0,0,103,"RFID Kaltwasser gefunden:");
		lcd_print(0,1,103,logstr);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'RFID gelesen    Bitte Select-->'");
	}
	sleep(5);
	lcd_clear();
	lcd_print(0,0,103,"Einlesen Kaltwasser temp..");
	lcd_print(0,1,103,"Taste Next -->");
	waitForEnter();
	//log("warte auf Taste.....");
	//getchar();
	//system("../bin/lcd_senso w");
	log("lese Temperatursensor aus..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese Probetemp. bitte warten...'");
	//sleep(5);
	//ret=get_temp (&ad);
	ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der Temperatur!!");
		lcd_clear();
		lcd_print(0,0,103,"Kaltwassertemp. nicht gelesen");
		return 1;
	} else {
		log("Temperatur erfolgreich ausgelesen:");
		sprintf(logstr,"Temperatur: %s°C",ad.k_temp);
		lcd_clear();
		lcd_print(0,0,103,"Kaltwassertemp. gelesen");
		lcd_print(0,1,103,logstr);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'Temp gelesen    Bitte Select-->'");
	}
	sleep(5);
	//Warmwasserwerte lesen
	lcd_print(0,1,103,"RFID Warmwasser -> Next");
	waitForEnter();
	//system("../bin/lcd_senso w");
	log("lese RFID Sensor Warmwasser aus");
	lcd_clear();
	lcd_print(0,0,103,"RFID wird gesucht..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese RFID Daten bitte warten...'");
	ret=get_rfid (&ad,'w'); //lese Kaltwasser RFID
	//ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der RFID-Daten!!");
		lcd_clear();
		lcd_print(0,0,103,"Fehler bei lesen der RFID-Werte..");
		return 1;
	} else {
		log("RFID erfolgreich ausgelesen:");
		sprintf(logstr,"RFID: %s",ad.w_rfid);
		lcd_clear();
		lcd_print(0,0,103,"RFID Warmwasser gefunden:");
		lcd_print(0,1,103,logstr);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'RFID gelesen    Bitte Select-->'");
	}
	sleep(5);
	lcd_clear();
	lcd_print(0,0,103,"Einlesen Warmwasser temp..");
	lcd_print(0,0,103,"Taste Next -->");
	waitForEnter();
	//log("warte auf Taste.....");
	//getchar();
	//system("../bin/lcd_senso w");
	log("lese Temperatursensor aus..");
	//system("../bin/lcd_senso c");
	//system("../bin/lcd_senso p 0 0 103 'lese Probetemp. bitte warten...'");
	//sleep(5);
	//ret=get_temp (&ad);
	ret=0;
	if ( ret != 0 ) {
		log("Fehler beim auslesen der Temperatur!!");
		lcd_clear();
		lcd_print(0,0,103,"Warmwassertemp. nicht gelesen");
		return 1;
	} else {
		log("Temperatur erfolgreich ausgelesen:");
		sprintf(logstr,"Temperatur: %s°C",ad.w_temp);
		lcd_clear();
		lcd_print(0,0,103,"Kaltwassertemp. gelesen");
		lcd_print(0,1,103,logstr);
		log(logstr);
		//system("../bin/lcd_senso c");
		//system("../bin/lcd_senso p 0 0 106 'Temp gelesen    Bitte Select-->'");
	}
	sleep(5);
	
	//write data to file
	lcd_clear();
	lcd_print(0,0,103,"Daten schreiben...");
	waitForEnter();
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
