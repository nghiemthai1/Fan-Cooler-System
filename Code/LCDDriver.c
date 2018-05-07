/*
 * LCDDriver.c
 *
 *  Created on: Oct 17, 2017
 *      Author: Russty
 */

/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/




#include "LCDDriver.h"

// LCD memory map for numeric digits
const char digit[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    {0xF7, 0x00}   /* "9" */
};

// LCD memory map for uppercase letters
const char alphabetBig[26][2] =
{
    {0xEF, 0x00},  /* "A" LCD segments a+b+c+e+f+g+m */
    {0xF1, 0x50},  /* "B" */
    {0x9C, 0x00},  /* "C" */
    {0xF0, 0x50},  /* "D" */
    {0x9F, 0x00},  /* "E" */
    {0x8F, 0x00},  /* "F" */
    {0xBD, 0x00},  /* "G" */
    {0x6F, 0x00},  /* "H" */
    {0x90, 0x50},  /* "I" */
    {0x78, 0x00},  /* "J" */
    {0x0E, 0x22},  /* "K" */
    {0x1C, 0x00},  /* "L" */
    {0x6C, 0xA0},  /* "M" */
    {0x6C, 0x82},  /* "N" */
    {0xFC, 0x00},  /* "O" */
    {0xCF, 0x00},  /* "P" */
    {0xFC, 0x02},  /* "Q" */
    {0xCF, 0x02},  /* "R" */
    {0xB7, 0x00},  /* "S" */
    {0x80, 0x50},  /* "T" */
    {0x7C, 0x00},  /* "U" */
    {0x0C, 0x28},  /* "V" */
    {0x6C, 0x0A},  /* "W" */
    {0x00, 0xAA},  /* "X" */
    {0x00, 0xB0},  /* "Y" */
    {0x90, 0x28}   /* "Z" */
};


const char alphabetSmall[26][2]=
{
     {0xFB, 0x00}, /* "a" */
     {0x3F, 0x00}, /* "b" */
     {0x1B, 0x00}, /* "c" */
     {0x7B, 0x00}, /* "d" */
     {0xDF, 0x00}, /* "e" */
     {0xCE, 0x00}, /* "f" */
     {0xF7, 0x08}, /* "g" */
     {0x2F, 0x00}, /* "h" */
     {0x00, 0x50}, /* "i" */
     {0x78, 0x00}, /* "j" */
     {0x00, 0x72}, /* "k" */
     {0x60, 0x00}, /* "l" */
     {0x2B, 0x10}, /* "m" */
     {0x0A, 0x10}, /* "n" */
     {0x3B, 0x00}, /* "o" */
     {0xC1, 0x50}, /* "p" */
     {0xFC, 0x02}, /* "q" */
     {0x01, 0x10}, /* "r" */
     {0xB7, 0x00}, /* "s" */
     {0x1F, 0x00}, /* "t" */
     {0x38, 0x00}, /* "u" */
     {0x08, 0x08}, /* "v" */
     {0x38, 0x10}, /* "w" */
     {0x00, 0xAA}, /* "x" */
     {0x71, 0x40}, /* "y" */
     {0x12, 0x08} /* "z" */
};



 void showChar(char c, int LCDposition)
 {

     if (c == ' ')
     {
         // Display space
         switch(LCDposition){
         case 1:
             LCDM10 = 0x0000;
             LCDM11 = 0x0000;
             break;
         case 2:
             LCDM6 = 0x0000;
             LCDM7 = 0x0000;
             break;
         case 3:
             LCDM4 = 0x0000;
             LCDM5 = 0x0000;
             break;
         case 4:
             LCDM19 = 0x0000;
             LCDM20 = 0x0000;
             break;
         case 5:
             LCDM15 = 0x0000;
             LCDM16 = 0x0000;
             break;
         case 6:
             LCDM8 = 0x0000;
             LCDM9 = 0x0000;
             break;
         }
     }
     else if (c >= '0' && c <= '9')
     {
         // Display digit
         switch(LCDposition){
         case 1:
             LCDM10 = digit[c-'0'][0];
             LCDM11 = digit[c-'0'][1];
             break;
         case 2:
             LCDM6 = digit[c-'0'][0];
             LCDM7 = digit[c-'0'][1];
             break;
         case 3:
             LCDM4 = digit[c-'0'][0];
             LCDM5 = digit[c-'0'][1];
             break;
         case 4:
             LCDM19 = digit[c-'0'][0];
             LCDM20 = digit[c-'0'][1];
             break;
         case 5:
             LCDM15 = digit[c-'0'][0];
             LCDM16 = digit[c-'0'][1];
             break;
         case 6:
             LCDM8 = digit[c-'0'][0];
             LCDM9 = digit[c-'0'][1];
             break;
         }
     }
     else if (c >= 'A' && c <= 'Z')
     {
         // Display uppercase alphabet
         //LCDMEMW[Position/2] = alphabetBig[c-65][0] | (alphabetBig[c-65][1] << 8);
         switch(LCDposition){
         case 1:
             LCDM10 = alphabetBig[c-65][0];
             LCDM11 = alphabetBig[c-65][1];
             break;
         case 2:
             LCDM6 = alphabetBig[c-65][0];
             LCDM7 = alphabetBig[c-65][1];
             break;
         case 3:
             LCDM4 = alphabetBig[c-65][0];
             LCDM5 = alphabetBig[c-65][1];
             break;
         case 4:
             LCDM19 = alphabetBig[c-65][0];
             LCDM20 = alphabetBig[c-65][1];
             break;
         case 5:
             LCDM15 = alphabetBig[c-65][0];
             LCDM16 = alphabetBig[c-65][1];
             break;
         case 6:
             LCDM8 = alphabetBig[c-65][0];
             LCDM9 = alphabetBig[c-65][1];
             break;
         }

     }
     else if (c >= 'a' && c <= 'z')
     {
         // Display lowercase alphabet
         switch(LCDposition){
         case 1:
             LCDM10 = alphabetSmall[c-'a'][0];
             LCDM11 = alphabetSmall[c-'a'][1];
             break;
         case 2:
             LCDM6 = alphabetSmall[c-'a'][0];
             LCDM7 = alphabetSmall[c-'a'][1];
             break;
         case 3:
             LCDM4 = alphabetSmall[c-'a'][0];
             LCDM5 = alphabetSmall[c-'a'][1];
             break;
         case 4:
             LCDM19 = alphabetSmall[c-'a'][0];
             LCDM20 = alphabetSmall[c-'a'][1];
             break;
         case 5:
             LCDM15 = alphabetSmall[c-'a'][0];
             LCDM16 = alphabetSmall[c-'a'][1];
             break;
         case 6:
             LCDM8 = alphabetSmall[c-'a'][0];
             LCDM9 = alphabetSmall[c-'a'][1];
             break;
         }
     }
     else
     {
         // Turn all segments on if character is not a space, digit, or uppercase letter
                  switch(LCDposition){
         case 1:
             LCDM10 = 0xFFFF;
             LCDM11 = 0xFFFF;
             break;
         case 2:
             LCDM6 = 0xFFFF;
             LCDM7 = 0xFFFF;
             break;
         case 3:
             LCDM4 = 0xFFFF;
             LCDM5 = 0xFFFF;
             break;
         case 4:
             LCDM19 = 0xFFFF;
             LCDM20 = 0xFFFF;
             break;
         case 5:
             LCDM15 = 0xFFFF;
             LCDM16 = 0xFFFF;
             break;
         case 6:
             LCDM8 = 0xFFFF;
             LCDM9 = 0xFFFF;
             break;
         }
     }
 }

 /*
 //Writes to screen 2
  void showCharB(char c, int position)
  {
      if (c == ' ')
      {
          // Display space
          LCDBMEMW[position/2] = 0;
      }
      else if (c >= '0' && c <= '9')
      {
          // Display digit
          LCDBMEMW[position/2] = (int)digit[c-48];
      }
      else if (c >= 'A' && c <= 'Z')
      {
          // Display alphabet
          LCDBMEMW[position/2] = alphabetBig[c-65][0] | (alphabetBig[c-65][1] << 8);
      }
      else
      {
          // Turn all segments on if character is not a space, digit, or uppercase letter
          LCDBMEMW[position/2] = 0xFFFF;
      }
  }
*/
