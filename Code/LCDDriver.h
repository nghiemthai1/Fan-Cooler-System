/*
 * LCDDriver.h
 *
 *  Created on: Oct 17, 2017
 *      Author: Russty
 */

#ifndef LCDDRIVER_H_
#define LCDDRIVER_H_

#include <msp430.h>

#define batEven 13
#define batOdd 17


extern const int position[6];
extern const char digit[10][2];
extern const char alphabetBig[26][2];
extern const char alphabetSmall[26][2];



void showChar(char c, int position);
void showCharB(char c, int position);

#endif /* LCDDRIVER_H_ */
