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
//******************************************************************************
//   MSP430FR6989
//      Milestone 2
//
//   Description: Code for a closed loop system that take in a goal temperature
//                to adjust the fan speed to keep the system temperature close
//                to the goal temperature.
//   ACLK = 32.768kHz, MCLK = SMCLK = default DCO~1MHz
//
//                MSP430
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |             P1.0|-->LED
//
//   Thai Nghiem and Ardit Pranvoku
//   Rowan University
//   November 2017
//   Built with CCSv4 and IAR Embedded Workbench Version: 4.21
//******************************************************************************

#include "msp430.h"
#include <LCDDriver.h>
#include <stdlib.h>

void LCDInit();
void TimerInit(void);
void UARTinit(void);
void ADC12init(void);
char convertToChar(int);

int adcMem_bit = 0; // each bit in the ADC12MEM0
int readinginputTemp = 0; // outside input temperature from the LM35
int inputTemp = 34; // initialize first input temperature
int adc_int = 0;  //will contain contents from ADC12MEM0
int adcArray[3];  //will contain digits of adc_int


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode

    //Initialization methods
    LCDInit();  //initialize LCD display
    TimerInit();  //initialize timer
    UARTinit();  //initialize UART
    ADC12init(); // initialize ADC12

    P1DIR |= BIT0;                          // Set P1.0 to output direction
    P1OUT &= ~BIT0;                         // Switch LED off
    P1SEL0 |= BIT0;                         // PWM output to LED P1.0
    P1SEL1 &= ~BIT0;                        //


    P2SEL0 |= BIT0 | BIT1;                    // USCI_A0 UART operation GREEN 2.1 WHITE 2.0
    P2SEL1 &= ~(BIT0 | BIT1);

    __enable_interrupt();
    __bis_SR_register(GIE);
    while (1)
    {
        __delay_cycles(1000);
        ADC12CTL0 |= ADC12ENC | ADC12SC;    // Start sampling/conversion
        __bis_SR_register(LPM0_bits | GIE); // LPM0, ADC12_ISR will force exit
        __no_operation();                   // For debugger
    }

}



//ADC 12 interrupt
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
    {
    case ADC12IV_NONE:        break;    // Vector  0:  No interrupt
    case ADC12IV_ADC12OVIFG:  break;    // Vector  2:  ADC12MEMx Overflow
    case ADC12IV_ADC12TOVIFG: break;    // Vector  4:  Conversion time overflow
    case ADC12IV_ADC12HIIFG:  break;    // Vector  6:  ADC12BHI
    case ADC12IV_ADC12LOIFG:  break;    // Vector  8:  ADC12BLO
    case ADC12IV_ADC12INIFG:  break;    // Vector 10:  ADC12BIN
    case ADC12IV_ADC12IFG0:             // Vector 12:  ADC12MEM0 Interrupt

        // Make the outside input temperature more accurate when taking it
        // out from ADC12MEM0
        if(inputTemp < 30)
        {
            readinginputTemp = (ADC12MEM0/10) - 6; //changes duty cycle
            adc_int = (ADC12MEM0/10) - 6;
        }
        if(inputTemp >= 30 && inputTemp <= 40)
        {
            readinginputTemp = (ADC12MEM0/10) - 7; //changes duty cycle
            adc_int = (ADC12MEM0/10) - 7;
        }
        else if(inputTemp >= 40 && inputTemp < 50)
        {
            readinginputTemp = (ADC12MEM0/10) - 8; //changes duty cycle
            adc_int = (ADC12MEM0/10) - 8;
        }
        else if(inputTemp >= 50)
        {
            readinginputTemp = (ADC12MEM0/10) - 10; //changes duty cycle
            adc_int = (ADC12MEM0/10) - 10;
        }
        //readinginputTemp = readinginputTemp * 10;
        //adc_int = adc_int * 10;

        // If the outside temp is more than 3 degrees warmer than the goal temp -> 100% duty cycle, fans at maximum speed.
        if(readinginputTemp > inputTemp + 3)
        {
            TA0CCR1 = 1000;
        }
        // If the outside temp is more than 3 degree colder than the goal temp -> 0% duty cycle, turn fan off.
        else if(readinginputTemp < inputTemp - 3)
        {
            TA0CCR1 = 0;
        }
        // Start using calculated piecewise functions to control the PWM duty cycles.
        // 50-100% duty cycle function.
        else if(inputTemp >= 37 && inputTemp < 38)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-16.67 + 691.67)) ;      // y = -16.67 *(Temp) + 691.67
            }
        }
        // 30-50% duty cycle function.
        else if(inputTemp >= 38 && inputTemp <= 40)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-6.25 + 290.63)) ;      // y = -6.25 *(Temp) + 290.63
            }
        }
        // 20-30% duty cycle function.
        else if(inputTemp >= 40 && inputTemp <= 42)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-7.14 + 322.14)) ;      // y = -7.14 *(Temp) + 322.14
            }
        }
        // 10-20% duty cycle function.
        else if(inputTemp >= 42 && inputTemp <= 45)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-3.57 + 171.07)) ;      // y = -3.57 *(Temp) + 171.07
            }
        }
        // 8-10% duty cycle function.
        else if(inputTemp >= 45 && inputTemp <= 47)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-0.97 + 55)) ;      // y = -0.95 *(Temp) + 52.95
            }
        }
        // 5-8% duty cycle function.
        else if(inputTemp >= 47 && inputTemp <= 62)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-1/5 + 17.50)) ;      // y = -1/5*(Temp) + 17.50
            }
        }
        // 0-5% duty cycle function.
        else if(inputTemp >= 62)
        {
            if(readinginputTemp >= inputTemp - 3 && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = (10 * ((inputTemp)*-0.68 + 47.53)) ;      // y = -0.68 (Temp) + 47.53
            }
        }
        //  The below block of code is used to make sure the readinginputTemp and the input is out of the range of
        //  the graph.
        else{
            if(readinginputTemp >= inputTemp && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = 200 + ((readinginputTemp - inputTemp) * 85 + (readinginputTemp - inputTemp));
            }
            else if( readinginputTemp >= inputTemp && readinginputTemp <= inputTemp + 3)
            {
                TA0CCR1 = 200 + ((readinginputTemp - inputTemp) * 6  - (readinginputTemp - inputTemp));
            }
        }

        adcMem_bit = 0;

        //iterates through 3 digit adc_int and puts each digit into adcArray in reverse order
        do{
            adcArray[adcMem_bit]=(adc_int%10);
            adc_int/=10;
            adcMem_bit++;
        }while(adc_int>0);


        //prints out characters to lcd screen
        showChar(convertToChar(adcArray[2]), 3);
        showChar(convertToChar(adcArray[1]), 4);
        showChar(convertToChar(adcArray[0]), 5);

        // Exit from LPM0 and continue executing main
        __bic_SR_register_on_exit(LPM0_bits);
        break;
    default: break;
    }
}

//inits lcd - Given by Russel
void LCDInit()
{
    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
        CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory
    //Turn LCD on
    LCDCCTL0 |= LCDON;
	//Initialize LCD display
    showChar('0',0);
    showChar('0',1);
    showChar('0',2);
    showChar('0',3);
    showChar('0',4);
    showChar('0',5);   
    showChar('C',6);  // Celsius sign
}


// Converting an interger to a character
char convertToChar(int numInput){
    char number;

    if(numInput == 0) // Turn number zero to a character
    {
        number = '0';
    }
    else if(numInput == 1) // Turn number one to a character
    {
        number = '1';
    }
    else if(numInput == 2)  // Turn number two to a character
    {
        number = '2';
    }
    else if(numInput == 3)  // Turn number three to a character
    {
        number = '3';
    }
    else if(numInput == 4)    // Turn number four to a character
    {
        number = '4';
    }
    else if(numInput == 5)     // Turn number five to a character
    {
        number = '5';
    }
    else if(numInput == 6)  // Turn number six to a character
    {
        number = '6';
    }
    else if(numInput == 7)  // Turn number seven to a character
    {
        number = '7';
    }
    else if(numInput == 8)  // Turn number eight to a character
    {
        number = '8';
    }
    else if(numInput == 9) // Turn number nine to a character
    {
        number = '9';
    }

    return number;

}


void TimerInit(){
    TA0CCTL1 = OUTMOD_7;    // Sets TA0CCR0 to set, sets TA0CCR1 to reset.
    TA0CTL = TASSEL_2 + MC_1 +TACLR ; // Uses SMCLK, UPMODE, and clears TA.
    TA0CCR0 = 1000; // Initialize the CCR0
    TA0CCR1 = 250;  // Initialize the CCR1
}

void UARTinit(){
    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
    // Baud Rate calculation
    // 8000000/(16*9600) = 52.083
    // Fractional portion = 0.083
    // User's Guide Table 21-4: UCBRSx = 0x04
    // UCBRFx = int ( (52.083-52)*16) = 1
    UCA0BR0 = 52;                             // 8000000/16/9600
    UCA0BR1 = 0x00;
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL0_H = 0;                             // Lock CS registers
}
void ADC12init(){
    // Configure ADC12
    REFCTL0=REFON + REFVSEL_2;           //Use a reference voltage of 2.5V
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;   // Sampling time, S&H=16, ADC12 on
    ADC12CTL1 = ADC12SHP;                // Use sampling timer, ADC12SC conversion start, clock=OSC
    ADC12CTL2 |= ADC12RES_2;             // 12-bit conversion results
    ADC12MCTL0 |= ADC12INCH_3;           // ADC input select
    ADC12IER0 |= ADC12IE0;               // Enable ADC conv complete interrupt
    ADC12CTL0 |= ADC12ENC;               //Enable conversion
    P1SEL0 |=BIT3;                      //Configure pin 1.3 for input channel 3
    P1SEL1 |=BIT3;
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)

{
    switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
    {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        while(!(UCA0IFG & UCTXIFG)); // Wait until buffer is ready
        UCA0TXBUF = UCA0RXBUF; // Transmit what is input
        inputTemp = UCA0RXBUF; // Assign the goal temperature
        break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    }
}
