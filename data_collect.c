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

static const char digits[] = {"0123456789"};
static const char requestDataLn1[] = {"device_id="};
static const char requestDataLn2[] = {"&time="};
static const char requestDataLn3[] = {"&temp="};
static const char requestDataLn4[] = {"&acc_x="};
static const char requestDataLn5[] = {"&acc_y="};
static const char requestDataLn6[] = {"&acc_z="};
static const char requestDataLn7[] = {"\r\n"};
static const char requestLn1[] = {"POST /collar/upload HTTP/1.1\r\nHost: 54.241.253.170\r\nUser-Agent: PawlerDevice1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: "};
static const char requestLn2[] = {"\r\n\r\n"};
char *length;
char *id;
char *time;
char *temp;
char *xacc;
char *yacc;
char *zacc;
char *pucDataRequest;
char *pucApiRequest;

#define LENGTH_PTR 0xE900
#define ID_PTR LENGTH_PTR + COUNTER_LENGTH
#define TIME_PTR ID_PTR + COUNTER_LENGTH
#define TEMP_PTR TIME_PTR + QUEUE_LENGTH
#define XACC_PTR TEMP_PTR + QUEUE_LENGTH
#define YACC_PTR XACC_PTR + QUEUE_LENGTH
#define ZACC_PTR YACC_PTR + QUEUE_LENGTH
#define DATA_REQ_PTR ZACC_PTR + QUEUE_LENGTH
#define TX_REQ_PTR DATA_REQ_PTR + DATA_LENGTH

#define COUNTER_LENGTH 16
#define QUEUE_LENGTH 50
#define DATA_LENGTH sizeof(requestDataLn1) + sizeof(requestDataLn2) + sizeof(requestDataLn3) + sizeof(requestDataLn4) + sizeof(requestDataLn5) + sizeof(requestDataLn6) + sizeof(requestDataLn7) + COUNTER_LENGTH + QUEUE_LENGTH + QUEUE_LENGTH + QUEUE_LENGTH + QUEUE_LENGTH + QUEUE_LENGTH
#define TX_LENGTH sizeof(requestLn1) + COUNTER_LENGTH + sizeof(requestLn2) + DATA_LENGTH

void StartMeasurement(void) {
	SetupSensors();

	length = (char *)LENGTH_PTR;
	id = (char *)ID_PTR;
	time = (char *)TIME_PTR;
	temp = (char *)TEMP_PTR;
	xacc = (char *)XACC_PTR;
	yacc = (char *)YACC_PTR;
	zacc = (char *)ZACC_PTR;
	pucDataRequest = (char *)DATA_REQ_PTR;
	pucApiRequest = (char *)TX_REQ_PTR;

	// open socket and connect
	short connected = 0;
	short reconnectTimer = 0;
	//long bytesRcv = 0;
	int optval, optlen;
	//unsigned long timeout = 10000;
	char buf[16];

	long errCode;
	sockaddr socketAddr;
	long socketRef = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//setsockopt(socketRef, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &timeout, sizeof(timeout));

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
	itoa(deviceId,id,10);

	short dataCount = 0;
	short sensorCounter = 0;
	int counter = 0;
	while (startMeasurement && errCode >= 0) {
		TakeADCMeas();

		if (dataCount == 0) {
			memset(buf,0,sizeof(buf));
			memset(time, 0, QUEUE_LENGTH);
			memset(temp, 0, QUEUE_LENGTH);
			memset(xacc, 0, QUEUE_LENGTH);
			memset(yacc, 0, QUEUE_LENGTH);
			memset(zacc, 0, QUEUE_LENGTH);
		}

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

		sensorCounter++;
		if (sensorCounter == 4) {
			sensorCounter = 0;

			memset(buf,0,sizeof(buf));
			itoa(counter,buf,10);
			if (dataCount == 0) {
				strcpy(time, buf);
			} else {
				strcat(time, ",");
				strcat(time, buf);
			}
			memset(buf,0,sizeof(buf));
			itoa(ADCResultT,buf,10);
			if (dataCount == 0) {
				strcpy(temp, buf);
			} else {
				strcat(temp, ",");
				strcat(temp, buf);
			}
			memset(buf,0,sizeof(buf));
			itoa(ADCResultX,buf,10);
			if (dataCount == 0) {
				strcpy(xacc, buf);
			} else {
				strcat(xacc, ",");
				strcat(xacc, buf);
			}
			memset(buf,0,sizeof(buf));
			itoa(ADCResultY,buf,10);
			if (dataCount == 0) {
				strcpy(yacc, buf);
			} else {
				strcat(yacc, ",");
				strcat(yacc, buf);
			}
			memset(buf,0,sizeof(buf));
			itoa(ADCResultZ,buf,10);
			if (dataCount == 0) {
				strcpy(zacc, buf);
			} else {
				strcat(zacc, ",");
				strcat(zacc, buf);
			}

			counter++;
			dataCount++;
			if (dataCount == 10) {
				dataCount = 0;

				// got measurement
				turnLedOn(3);
				Delay();
				Delay();
				Delay();
				Delay();

				memset(length,0,COUNTER_LENGTH);
				memset(pucDataRequest,0,DATA_LENGTH);
				memset(pucApiRequest,0,TX_LENGTH);

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

				itoa(strlen(pucDataRequest)-2,length,10);
				strcpy(pucApiRequest, requestLn1);
				strcat(pucApiRequest, length);
				strcat(pucApiRequest, requestLn2);
				strcat(pucApiRequest, pucDataRequest);

				send(socketRef, pucApiRequest, strlen(pucApiRequest), 0);
				closesocket(socketRef);

				reconnectTimer = 0;
				//connected =  getsockopt(socketRef, SOL_SOCKET, SOCKOPT_NONBLOCK , &optval, (socklen_t*)&optlen);
				while (reconnectTimer < 15) {
					Delay();
					Delay();
					Delay();
					Delay();
					socketRef = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					errCode = connect(socketRef, &socketAddr, sizeof(socketAddr));
					if (errCode >= 0) {
						connected = 1;
						break;
					} else
						connected = -1;
					closesocket(socketRef);
					reconnectTimer++;
				}

				turnLedOff(3);
			}
		}
	}

	if (connected)
		closesocket(socketRef);

	ShutDownSensors();
}

char *itoa(int n, char *s, int b) {

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
