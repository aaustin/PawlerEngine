#include "msp430fr5739.h"
#include "board.h"
#include "cc3000.h"
#include "data_collect.h"

volatile unsigned short smartConfigFinished = 0;
volatile unsigned short CC3000Connected = 0;
volatile unsigned short DHCPset = 0;
volatile unsigned short startMeasurement = 0;
volatile unsigned short startSmartConfig = 0;

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    SystemInit(); // init all board related pins

    initCC3000(); // init the wifi unit
	
    while(1) {
    	if (startSmartConfig) {
    		StartSmartConfig();
    	}

    	if (startMeasurement) {
    		StartMeasurement();
    	}

    	turnLedOn(1);
		Delay();
		Delay();
		turnLedOff(1);
		Delay();
		Delay();
    }

	return 0;
}


