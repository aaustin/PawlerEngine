/*
 * cc3000.h
 *
 *  Created on: Apr 5, 2013
 *      Author: Alex
 */

#ifndef CC3000_H_
#define CC3000_H_

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length);

void initCC3000(void);
int initDriver(void);
void StartSmartConfig(void);

char *sendDriverPatch(unsigned long *Length);
char *sendBootLoaderPatch(unsigned long *Length);
char *sendWLFWPatch(unsigned long *Length);

#endif /* CC3000_H_ */
