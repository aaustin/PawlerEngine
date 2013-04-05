/*
 * cc3000.c
 *
 *  Created on: Apr 5, 2013
 *      Author: Alex
 */

#include "board.h"
#include "wlan.h"
#include "evnt_handler.h"

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length);

void initCC3000(void);
int initDriver(void);
void StartSmartConfig(void) {
	// Reset all the previous configuration
	wlan_ioctl_set_connection_policy(0, 0, 0);
	wlan_ioctl_del_profile(255);

	//Wait until CC3000 is disconnected
	while (ulCC3000Connected == 1) {
		__delay_cycles(100);
		hci_unsolicited_event_handler();
	}

	// Start smart configuration process
	wlan_smart_config_set_prefix(NULL);

	// Start the Smart config process with AES disabled
    wlan_smart_config_start(0);

	// Wait for Smart config finished
    turnLedOn(6);
	while (ulSmartConfigFinished == 0) {
		Delay();
		turnLedOn(6);
		Delay();
		turnLedOff(6);
	}
	turnLedOff(6);

	// Configure to connect automatically to the AP retrieved in the
	// Smart config process
	if (ulSmartConfigFinished == 1) {
		wlan_ioctl_set_connection_policy(0, 0, 1);

		// reset the CC3000
		wlan_stop();
		Delay();
		wlan_start(0);

		// Mask out all non-required events
		wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
	}
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
