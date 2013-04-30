/*
 * cc3000.c
 *
 *  Created on: Apr 5, 2013
 *      Author: Alex
 */

#include "msp430fr5739.h"
#include "cc3000.h"
#include "board.h"
#include "wlan.h"
#include "evnt_handler.h"
#include "spi.h"
#include "netapp.h"

extern unsigned short smartConfigFinished;
extern volatile unsigned short CC3000Connected;
extern volatile unsigned short DHCPset;
extern volatile unsigned short startSmartConfig;

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length) {
    if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE) {
    	smartConfigFinished = 1;
        turnLedOn(8);
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_INIT) {
    	turnLedOn(8);
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT) {
    	CC3000Connected = 1;
    	turnLedOn(7);
    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT) {
    	CC3000Connected = 0;
    	DHCPset = 0;
    	turnAllLedOff();

    }

    if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP) {
    	DHCPset = 1;
        turnLedOn(6);
    }
}

void initCC3000(void) {
 	// init SPI_CS_PORT s
	// Configure the SPI CS to be on P1.3
	P1OUT |= BIT3;
	P1DIR |= BIT3;
	P1SEL1 &= ~BIT3;
	P1SEL0 &= ~BIT3;

    // P4.1 - WLAN enable full DS
	P4OUT &= ~BIT1;
	P4DIR |= BIT1;
	P4SEL1 &= ~BIT1;
	P4SEL0 &= ~BIT1;

    // Configure SPI IRQ line on P2.3
	P2DIR  &= (~BIT3);
	P2SEL1 &= ~BIT3;
	P2SEL0 &= ~BIT3;

	initDriver();
}

int initDriver(void) {
   //init all layers
	init_spi();

	// this is done for debug purpose only

	// WLAN On API Implementation
	wlan_init(CC3000_UsynchCallback, sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch, ReadWlanInterruptPin, WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);

	// Trigger a WLAN device
	wlan_start(0);


	// Mask out all non-required events from CC3000
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

	//unsolicicted_events_timer_init();

	// CC3000 has been initialized
	//setCC3000MachineState(CC3000_INIT);

	return(0);
}
void StartSmartConfig(void) {
	smartConfigFinished = 0;
	// Reset all the previous configuration
	wlan_ioctl_set_connection_policy(0, 0, 0);
	wlan_ioctl_del_profile(255);

	//Wait until CC3000 is disconnected
	while (CC3000Connected == 1) {
		__delay_cycles(100);
		hci_unsolicited_event_handler();
	}

	// Start smart configuration process
	wlan_smart_config_set_prefix(NULL);

	// Start the Smart config process with AES disabled
    wlan_smart_config_start(0);

	// Wait for Smart config finished
    turnLedOn(6);
	while (smartConfigFinished == 0 && startSmartConfig == 1) {
		Delay();
		turnLedOn(6);
		Delay();
		turnLedOff(6);
	}
	turnLedOff(6);

	// Configure to connect automatically to the AP retrieved in the
	// Smart config process
	if (smartConfigFinished == 1) {
		wlan_ioctl_set_connection_policy(0, 0, 1);

		// reset the CC3000
		wlan_stop();
		Delay();
		wlan_start(0);

		// Mask out all non-required events
		wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
	}

	startSmartConfig = 0;
}

char *sendDriverPatch(unsigned long *Length) {
    *Length = 0;
    return NULL;
}
char *sendBootLoaderPatch(unsigned long *Length) {
    *Length = 0;
    return NULL;
}
char *sendWLFWPatch(unsigned long *Length) {
    *Length = 0;
    return NULL;
}
