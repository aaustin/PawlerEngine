/*
 * board.c
 *
 *  Created on: Apr 5, 2013
 *      Author: Alex
 */
#include "msp430fr5739.h"
#include "board.h"
#include "cc3000.h"

volatile unsigned int ADCResultX = 0;
volatile unsigned int ADCResultY = 0;
volatile unsigned int ADCResultZ = 0;
volatile unsigned int ADCResultT = 0;
volatile unsigned int measurementComplete = 0;

extern volatile unsigned short DHCPset;
extern volatile unsigned short startMeasurement;
extern volatile unsigned short startSmartConfig;

void SystemInit(void) {
	// Startup clock system in max. DCO setting ~8MHz
	// This value is closer to 10MHz on untrimmed parts
	CSCTL0 = CSKEY;                          // Unlock register
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
	CSCTL1 &= ~DCORSEL;							// make sure at lowest range
	CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
	CSCTL0_H = 0xFF;                          // Lock Register

	// Turn off temp.
	REFCTL0 |= REFTCOFF;
	REFCTL0 &= ~REFON;

	// Enable switch 1
	P4OUT |= BIT0;                      // Configure pullup resistor
	P4DIR &= ~(BIT0);                  // Direction = input
	P4REN |= BIT0;                     // Enable pullup resistor
	P4IES &= ~(BIT0);                    // P4.0 Lo/Hi edge interrupt
	P4IE = BIT0;                         // P4.0 interrupt enabled
	P4IFG = 0;                                // P4 IFG cleared

	// Enable LEDs
	P3OUT &= ~(BIT6+BIT7+BIT5+BIT4);
	P3DIR |= BIT6+BIT7+BIT5+BIT4;
	PJOUT &= ~(BIT0+BIT1+BIT2+BIT3);
	PJDIR |= BIT0 +BIT1+BIT2+BIT3;

	// Terminate Unused GPIOs
	// P1.0 - P1.6 is unused
	P1OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);
	P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);
	P1REN |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);

	// P1.4 is used as input from NTC voltage divider
	// Set it to output low
	P1OUT &= ~BIT4;
	P1DIR |= BIT4;

	// P2.2 - P2.6 is unused
	P2OUT &= ~(BIT2 + BIT4 + BIT5 + BIT6);
	P2DIR &= ~(BIT2 + BIT4 + BIT5 + BIT6);
	P2REN |= (BIT2 + BIT4 + BIT5 + BIT6);


	// P2.7 is used to power the voltage divider for the NTC thermistor
	P2OUT &= ~BIT7;
	P2DIR |= BIT7;

	// P3.0,P3.1 and P3.2 are accelerometer inputs
	P3OUT &= ~(BIT0 + BIT1 + BIT2);
	P3DIR &= ~(BIT0 + BIT1 + BIT2);
	P3REN |= BIT0 + BIT1 + BIT2;

	// PJ.0,1,2,3 are used as LEDs
	// crystal pins for XT1 are unused
	PJOUT &= ~(BIT4+BIT5);
	PJDIR &= ~(BIT4+BIT5);
	PJREN |= BIT4 + BIT5;
}

#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void) {
	turnAllLedOff();
	switch(__even_in_range(P4IV,P4IV_P4IFG1)) {
		case P4IV_P4IFG0:
			DisableSwitches();
			StartDebounceTimer(); // Reenable switches after debounce

			if (DHCPset) {
				// start sensor collection
				if (startMeasurement) {
					startMeasurement = 0;
				} else {
					startMeasurement = 1;
				}
			} else {
				if (startSmartConfig) {
					startSmartConfig = 0;
				} else {
					startSmartConfig = 1;
				}
			}
			break;
		default:
			break;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void) {
	TA0CCTL0 = 0;
	TA0CTL = 0;
	EnableSwitches();
}

void turnAllLedOff(void) {
	// P3.4- P3.7 are set as output, low
	P3OUT &= ~(BIT4 + BIT5 + BIT6 + BIT7);
	P3DIR |= BIT4 + BIT5 + BIT6 + BIT7;
	// PJ.0,1,2,3 are set as output, low
	PJOUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);
	PJDIR |= BIT0 + BIT1 + BIT2 + BIT3;
}

void turnLedOn(int ledNum) {
	switch(ledNum) {
		case 1:
			PJOUT |= (BIT0);
			break;
		case 2:
			PJOUT |= (BIT1);
			break;
		case 3:
			PJOUT |= (BIT2);
			break;
		case 4:
			PJOUT |= (BIT3);
			break;
		case 5:
			P3OUT |= (BIT4);
			break;
		case 6:
			P3OUT |= (BIT5);
			break;
		case 7:
			P3OUT |= (BIT6);
			break;
		case 8:
			P3OUT |= (BIT7);
			break;
	}
}
void turnLedOff(int ledNum) {
	switch(ledNum) {
		case 1:
			PJOUT &= ~(BIT0);
			break;
		case 2:
			PJOUT &= ~(BIT1);
			break;
		case 3:
			PJOUT &= ~(BIT2);
			break;
		case 4:
			PJOUT &= ~(BIT3);
			break;
		case 5:
			P3OUT &= ~(BIT4);
			break;
		case 6:
			P3OUT &= ~(BIT5);
			break;
		case 7:
			P3OUT &= ~(BIT6);
			break;
		case 8:
			P3OUT &= ~(BIT7);
			break;
	}
}

void TakeADCMeas(void) {
	while (ADC10CTL1 & BUSY);
	measurementComplete = 0;
	ADC10CTL0 |= ADC10ENC | ADC10SC;
}

void SetupSensors(void) {
	// setup accel pins
	P3SEL0 |= BIT0 + BIT1 + BIT2;    //Enable A/D channel inputs
	P3SEL1 |= BIT0 + BIT1 + BIT2;
	P3DIR &= ~(BIT0 + BIT1 + BIT2);
	P2DIR |= BIT7;              //Enable ACC_POWER
	P2OUT |= BIT7;

	// setup temp measurement
	P1SEL1 |= BIT4;
	P1SEL0 |= BIT4;

	// Allow the accelerometer to settle before sampling any data
	__delay_cycles(200000);

	//Single channel, once,
	ADC10CTL0 &= ~ADC10ENC;                        // Ensure ENC is clear
	ADC10CTL0 = ADC10ON + ADC10SHT_5;
	ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_1 + ADC10SSEL_0;
	ADC10CTL2 = ADC10RES;
	ADC10MCTL0 = ADC10SREF_0; //set the A/D ref voltage
	ADC10IV = 0x00;  // Clear all ADC12 channel int flags
	ADC10IE |= ADC10IE0;
}

void ShutDownSensors(void) {
	P3SEL0 &= ~(BIT0 + BIT1 + BIT2);
	P3SEL1 &= ~(BIT0 + BIT1 + BIT2);
	P3DIR &= ~(BIT0 + BIT1 + BIT2);

	P2DIR &= ~BIT7;
	P2OUT &= ~BIT7;

	P1SEL1 &= ~BIT4;
	P1SEL0 &= ~BIT4;

	ADC10CTL0 &= ~(ADC10ENC + ADC10ON);
	ADC10IE &= ~ADC10IE0;
	ADC10IFG = 0;
}

/**********************************************************************//**
 * @brief  ADC10 ISR for MODE3 and MODE4
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	switch(__even_in_range(ADC10IV,ADC10IV_ADC10IFG)) {
		case ADC10IV_ADC10IFG:
			if (ADC10MCTL0 & ADC10INCH_12) {
				ADCResultX = ADC10MEM0;
			} else if (ADC10MCTL0 & ADC10INCH_13) {
				ADCResultY = ADC10MEM0;
			} else if (ADC10MCTL0 & ADC10INCH_14) {
				ADCResultZ = ADC10MEM0;
			} else if (ADC10MCTL0 & ADC10INCH_4) {
				ADCResultT = ADC10MEM0;
				measurementComplete = 1;
			}
            break;
		default: break;
	}
}

void DisableSwitches(void) {
	// disable switches
	P4IFG = 0;                                // P4 IFG cleared
	P4IE &= ~(BIT0+BIT1);                     // P4.0 interrupt disabled
	P4IFG = 0;
}

void EnableSwitches(void) {
	P4IFG = 0;                                // P4 IFG cleared
	P4IE = BIT0+BIT1;                         // P4.0 interrupt enabled
}

void StartDebounceTimer(void) {
	// default delay = 0
	// Debounce time = 1500* 1/8000 = ~200ms
	TA0CCTL0 = CCIE;                          // TACCR0 interrupt enabled
	TA0CCR0 = 1500;
	TA0CTL = TASSEL_1 + MC_1;                 // ACLK, up mode
}

long ReadWlanInterruptPin(void) {
    return (P2IN & BIT3);
}
void WlanInterruptEnable(void) {
    __bis_SR_register(GIE);
    P2IES |= BIT3;
    P2IE |= BIT3;
}
void WlanInterruptDisable(void) {
	P2IE &= ~BIT3;
}
void WriteWlanPin(unsigned char val) {
	if (val) {
		P4OUT |= BIT1;
    } else {
    	P4OUT &= ~BIT1;
    }
}

void Delay(void) {
	__delay_cycles(250000);
	__no_operation();
}
