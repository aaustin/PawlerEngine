/*
 * board.h
 *
 *  Created on: Apr 5, 2013
 *      Author: Alex
 */

#ifndef BOARD_H_
#define BOARD_H_

void SystemInit(void);

void turnAllLedOff(void);
void turnLedOn(int ledNum);
void turnLedOff(int ledNum);

void TakeADCMeas(void);
void SetupSensors(void);
void ShutDownSensors(void);

void DisableSwitches(void);
void EnableSwitches(void);
void StartDebounceTimer(void);

long ReadWlanInterruptPin(void);
void WlanInterruptEnable(void);
void WlanInterruptDisable(void);
void WriteWlanPin(unsigned char val);

void Delay(void);

#endif /* BOARD_H_ */
