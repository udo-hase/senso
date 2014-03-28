/*
 * main.h
 *
 *  Created on: 26.09.2013
 *      Author: alexs
 */

#ifndef MAIN_H_
#define MAIN_H_

extern uint8_t debug;
#define DEBUG 1


#endif /* MAIN_H_ */

void lcd_init(void);
void lcd_clear(void);
void lcd_print(int,int,int,char*);
void waitForEnter(void); 
