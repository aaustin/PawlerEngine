/*
 * data_collect.c
 *
 *  Created on: Apr 6, 2013
 *      Author: Alex
 */
#include "data_collect.h"
#include "socket.h"
#include "string.h"
#include "board.h"

extern volatile unsigned short measurementComplete;
extern volatile unsigned int ADCResultX;
extern volatile unsigned int ADCResultY;
extern volatile unsigned int ADCResultZ;
extern volatile unsigned int ADCResultT;
extern volatile unsigned short startMeasurement;

void StartMeasurement(void) {
	SetupSensors();

	// open socket and connect
	short connected = 0;
	long errCode;
	sockaddr socketAddr;
	long socketRef = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int port = 80;
	socketAddr.sa_data[0] = (port & 0xFF00)>> 8;
	socketAddr.sa_data[1] = (port & 0x00FF);

	socketAddr.sa_data[2] = 54;
	socketAddr.sa_data[3] = 241;
	socketAddr.sa_data[4] = 253;
	socketAddr.sa_data[5] = 170;

	errCode = connect(socketRef, &socketAddr, sizeof(socketAddr));
	if (errCode >= 0)
		connected = 1;

	// prepare flexible packet for send

	int deviceId = 1;
	char length[10];
	char id[10];
	char time[10];
	char temp[10];
	char xacc[10];
	char yacc[10];
	char zacc[10];
	itoa(deviceId,id,10);
	char requestDataLn1[] = {"device_id="};
	char requestDataLn2[] = {"&time="};
	char requestDataLn3[] = {"&temp="};
	char requestDataLn4[] = {"&acc_x="};
	char requestDataLn5[] = {"&acc_y="};
	char requestDataLn6[] = {"&acc_z="};
	char requestDataLn7[] = {"\r\n"};
	char pucDataRequest[sizeof(requestDataLn1)
				                    + sizeof(requestDataLn2)
				                    + sizeof(requestDataLn3)
				                    + sizeof(requestDataLn4)
				                    + sizeof(requestDataLn5)
				                    + sizeof(requestDataLn6)
				                    + sizeof(requestDataLn7)
				                    + sizeof(id)
				                    + sizeof(time)
				                    + sizeof(temp)
				                    + sizeof(xacc)
				                    + sizeof(yacc)
				                    + sizeof(zacc)];
	char requestLn1[] = {"POST /collar/upload HTTP/1.1\r\nHost: 54.241.253.170\r\nUser-Agent: PawlerDevice1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: "};
	char requestLn2[] = {"\r\n\r\n"};
	char pucApiRequest[sizeof(requestLn1)
	                   + sizeof(length)
	                   + sizeof(requestLn2)
	                   + sizeof(pucDataRequest)];

	while (startMeasurement && errCode >= 0) {
		TakeADCMeas();

		while (!measurementComplete) {
			turnLedOn(2);
			Delay();
			Delay();
			Delay();
			Delay();
			turnLedOff(2);
			Delay();
			Delay();
			Delay();
			Delay();
		}

		// got measurement
		turnLedOn(3);
		Delay();
		Delay();
		Delay();
		Delay();

		memset(time,0,sizeof(time));
		memset(temp,0,sizeof(temp));
		memset(xacc,0,sizeof(xacc));
		memset(yacc,0,sizeof(yacc));
		memset(zacc,0,sizeof(zacc));
		memset(pucDataRequest,0,sizeof(pucDataRequest));
		memset(pucApiRequest,0,sizeof(pucApiRequest));

		itoa(1000,time,10);
		itoa(ADCResultT,temp,10);
		itoa(ADCResultX,xacc,10);
		itoa(ADCResultY,yacc,10);
		itoa(ADCResultZ,zacc,10);

		strcpy(pucDataRequest, requestDataLn1);
		strcat(pucDataRequest, id);
		strcat(pucDataRequest, requestDataLn2);
		strcat(pucDataRequest, time);
		strcat(pucDataRequest, requestDataLn3);
		strcat(pucDataRequest, temp);
		strcat(pucDataRequest, requestDataLn4);
		strcat(pucDataRequest, xacc);
		strcat(pucDataRequest, requestDataLn5);
		strcat(pucDataRequest, yacc);
		strcat(pucDataRequest, requestDataLn6);
		strcat(pucDataRequest, zacc);
		strcat(pucDataRequest, requestDataLn7);
		strcat(pucDataRequest, '\0');

		itoa(strlen(pucDataRequest)-2,length,10);
		strcpy(pucApiRequest, requestLn1);
		strcat(pucApiRequest, length);
		strcat(pucApiRequest, requestLn2);
		strcat(pucApiRequest, pucDataRequest);
		strcat(pucApiRequest, '\0');

		send(socketRef, pucApiRequest, strlen(pucApiRequest), 0);

		turnLedOff(3);
	}

	if (connected)
		closesocket(socketRef);

	ShutDownSensors();
}

char *itoa(int n, char *s, int b) {
	static char digits[] = "0123456789";
	int i=0, sign;

	if ((sign = n) < 0)
		n = -n;

	do {
		s[i++] = digits[n % b];
	} while ((n /= b) > 0);

	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';

	return strrev(s);
}


char *strrev(char *str) {
	char *p1, *p2;

	if (!str || !*str)
		return str;

	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}

	return str;
}
