#include "stdio.h"
#include "string.h"
#include <unistd.h>

#define SENSOR "/sys/bus/w1/devices/28-000004d08e2d/w1_slave"

int main (){
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
		printf("-->%d\n",h);
		fgets(yes[h],10,w1_slave); yes[h][3]=0x00;
		//read 4 lines of rubish
		for(i=0;i<3;i++){
			fgets(tmp,10,w1_slave);
		}
		//read the temp
		printf("-->%d\n",h);
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
	
printf("\n\n");
	printf("yes[%d]-->%s<--\n",h,yes[h]);
	printf("temp[%d]-->%s<--\n",h,temp[h]);
}
