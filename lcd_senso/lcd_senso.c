/*
 * lcd-adafruit.c:
 *	Text-based LCD driver test code
 *	This is designed to drive the Adafruit RGB LCD Plate
 *	with the additional 5 buttons for the Raspberry Pi
 *
 * Copyright (c) 2012-2013 Gordon Henderson.
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 * 
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <wiringPi.h>
#include <mcp23017.h>
#include <lcd.h>

#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif


// Defines for the Adafruit Pi LCD interface board

#define	AF_BASE		100
#define	AF_RED		(AF_BASE + 6)
#define	AF_GREEN	(AF_BASE + 7)
#define	AF_BLUE		(AF_BASE + 8)

#define	AF_E		(AF_BASE + 13)
#define	AF_RW		(AF_BASE + 14)
#define	AF_RS		(AF_BASE + 15)

#define	AF_DB4		(AF_BASE + 12)
#define	AF_DB5		(AF_BASE + 11)
#define	AF_DB6		(AF_BASE + 10)
#define	AF_DB7		(AF_BASE +  9)

#define	AF_SELECT	(AF_BASE +  0)
#define	AF_RIGHT	(AF_BASE +  1)
#define	AF_DOWN		(AF_BASE +  2)
#define	AF_UP		(AF_BASE +  3)
#define	AF_LEFT		(AF_BASE +  4)


// Global lcd handle:

static int lcdHandle ;

/*
 * usage:
 *********************************************************************************
 */

int usage (const char *progName)
{
  fprintf (stderr, "Usage: %s function\n", progName) ;
  fprintf (stderr, "      - i  				--> init the Display\n");
  fprintf (stderr, "	  - c  				--> clear Display and set to collor 103\n");
  fprintf (stderr, "      - s col 			--> set the Display Color to <col>\n");
  fprintf (stderr, "      - p x y col str 	--> print <str> in col <col> at pos x/y\n");
  fprintf (stderr, "      - w 				--> wait for Enter Taste\n");
  fprintf (stderr, "      - m col str       --> gibt message in color aus\n");
  return EXIT_FAILURE ;
}


/*
 * scrollMessage:
 *********************************************************************************
 */

//static const char *message =
 // "                    "
  //"Wiring Pi by Gordon Henderson. HTTP://WIRINGPI.COM/"
  //"                    " ;

void scrollMessage (int line, int width, char *message)
{
  char buf [32] ;
  static int position = 0 ;
  static int timer = 0 ;

  if (millis () < timer)
    return ;

  timer = millis () + 20000 ;

  strncpy (buf, &message [position], width) ;
  buf [width] = 0 ;
  lcdPosition (lcdHandle, 0, line) ;
  lcdPuts (lcdHandle, buf) ;

  if (++position == (strlen (message) - width))
    position = 0 ;
}


/*
 * setBacklightColour:
 *	The colour outputs are inverted.
 *********************************************************************************
 */

static void setBacklightColour (int colour)
{
  colour &= 7 ;

  digitalWrite (AF_RED,   !(colour & 1)) ;
  digitalWrite (AF_GREEN, !(colour & 2)) ;
  digitalWrite (AF_BLUE,  !(colour & 4)) ;
}


/*
 * adafruitLCDSetup:
 *	Setup the Adafruit board by making sure the additional pins are
 *	set to the correct modes, etc.
 *********************************************************************************
 */

static void adafruitLCDSetup (int colour)
{
  int i ;

//	Backlight LEDs

  pinMode (AF_RED,   OUTPUT) ;
  pinMode (AF_GREEN, OUTPUT) ;
  pinMode (AF_BLUE,  OUTPUT) ;
  setBacklightColour (colour) ;

//	Input buttons

  for (i = 0 ; i <= 4 ; ++i)
  {
    pinMode (AF_BASE + i, INPUT) ;
    pullUpDnControl (AF_BASE + i, PUD_UP) ;	// Enable pull-ups, switches close to 0v
  }

// Control signals

  pinMode (AF_RW, OUTPUT) ; digitalWrite (AF_RW, LOW) ;	// Not used with wiringPi - always in write mode

// The other control pins are initialised with lcdInit ()

  lcdHandle = lcdInit (2, 16, 4, AF_RS, AF_E, AF_DB4,AF_DB5,AF_DB6,AF_DB7, 0,0,0,0) ;

  if (lcdHandle < 0)
  {
    fprintf (stderr, "lcdInit failed\n") ;
    exit (EXIT_FAILURE) ;
  }
}


/*
 * waitForEnter:
 *	On the Adafruit display, wait for the select button
 *********************************************************************************
 */

static void waitForEnter (void)
{
  printf ("Press SELECT to continue: ") ; fflush (stdout) ;

  while (digitalRead (AF_SELECT) == HIGH)	// Wait for push
    delay (1) ;

  while (digitalRead (AF_SELECT) == LOW)	// Wait for release
    delay (1) ;

  printf ("OK\n") ;
}


/*
 * The works
 *********************************************************************************
 */

int lcd_init(){
  wiringPiSetupSys () ;
  mcp23017Setup (AF_BASE, 0x20) ;

  adafruitLCDSetup (105) ; //setze Farbe

  lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "Warmwasserproben") ;
  lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, " Daten Aufnahme ");
  sleep (2);
  lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "Start --->      ") ;
  lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "Press Select... ");
  waitForEnter () ;
  lcdClear (lcdHandle) ;
  adafruitLCDSetup (103) ; //setze Farbe
  
  return(0);
}

int lcd_clear(){
		wiringPiSetupSys () ;
		mcp23017Setup (AF_BASE, 0x20) ;
		adafruitLCDSetup (104);
		lcdClear (lcdHandle) ;
		return(0);
}

int lcd_print(int x,int y,int col,char *str){
		wiringPiSetupSys () ;
		mcp23017Setup (AF_BASE, 0x20) ;
		adafruitLCDSetup (col);
		//str[16]=0x00; //ende des Strings bei Pos. 16 :)
		printf("x=%d;y=%d;str='%s'\n",x,y,str);
		lcdPosition (lcdHandle, x, y) ; lcdPuts (lcdHandle, str) ;
		return(0);
}

int main (int argc, char *argv[]){
  int color ;
  int x;
  int y;
  char str[50];
  int waitForRelease = FALSE ;
  char Fkt;

  if (argc < 2)
    return usage (argv [0]) ;

  Fkt = argv[1][0];
  // Auswahl der Funktion
  switch(Fkt){
	case 'i' :  lcd_init();
				break;
	case 'c' :  lcd_clear();
				break;
	case 's' : 	wiringPiSetupSys () ;
				mcp23017Setup (AF_BASE, 0x20) ;
				color=atoi(argv[2]);
				adafruitLCDSetup (color);
				break;  
	case 'p' :	x=atoi(argv[2]);y=atoi(argv[3]);color=atoi(argv[4]);
				strcpy(str,argv[2]);
				printf("str->%s\n",argv[5]);
				lcd_print(x,y,color,argv[5]);
				break;
	case 'm' :  color=atoi(argv[2]);
				strcpy(str,argv[3]);
				wiringPiSetupSys () ;
				mcp23017Setup (AF_BASE, 0x20) ;
				adafruitLCDSetup (color);
				scrollMessage(0,strlen(str),str);
				break;
	case 'w' :  mcp23017Setup (AF_BASE, 0x20) ;
				waitForEnter () ;
				break;
	default  :  printf("Funktion nicht definiert!!!\n");
  }

return(0);
}

