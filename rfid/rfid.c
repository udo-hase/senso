#include <wiringPiSPI.h>

main(){

int ret=0;
int rfid=0;
int len=18;
char data[100];


rfid=wiringPiSPISetup (0,5000);
while (1){
	wiringPiSPIDataRW (0,data,16);
printf("RFID-->%s\n",data);
sleep(2);
}

}
