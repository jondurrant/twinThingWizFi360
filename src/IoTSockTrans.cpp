/*
 * IoTSockTrans.cpp
 *
 *  Created on: 11 Aug 2022
 *      Author: jondurrant
 */

#include "IoTSockTrans.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include <errno.h>

#define DEBUG_LINE 25

IoTSockTrans::IoTSockTrans() {

}

IoTSockTrans::~IoTSockTrans() {
	// NOP
}

/***
 * Required by CoreMQTT returns time in ms
 * @return
 */
uint32_t IoTSockTrans::getCurrentTime(){
	return to_ms_since_boot(get_absolute_time ());
}

/***
 * Send bytes through socket
 * @param pNetworkContext - Network context object from MQTT
 * @param pBuffer - Buffer to send from
 * @param bytesToSend - number of bytes to send
 * @return number of bytes sent
 */
int32_t IoTSockTrans::transSend(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend){
	uint32_t dataOut;

	//IoTSockTrans::debugPrintBuffer("SEND PLAIN", pBuffer, bytesToSend);

	dataOut = iotSocketSend(xSock, (const unsigned char*)pBuffer, bytesToSend);
	if (dataOut != bytesToSend){
		LogError(("Send failed 0x%X\n", dataOut));
	}
	return dataOut;
}


/***
 * Send
 * @param pNetworkContext
 * @param pBuffer
 * @param bytesToRecv
 * @return
 */
int32_t IoTSockTrans::transRead(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv){
	int32_t dataIn=0;

	dataIn =  iotSocketRecv(xSock, pBuffer, bytesToRecv);

	if (dataIn < 0){
		switch(dataIn){
			case IOT_SOCKET_EAGAIN:
				dataIn=0;
				break;
		}
	}

	//if (dataIn > 0){
	//	IoTSockTrans::debugPrintBuffer("READ PLAIN", pBuffer, dataIn);
	//}
	return dataIn;
}


/***
 * Static function to send data through socket from buffer
 * @param pNetworkContext - Used to locate the IoTSockTrans object to use
 * @param pBuffer - Buffer of data to send
 * @param bytesToSend - number of bytes to send
 * @return number of bytes sent
 */
int32_t IoTSockTrans::staticSend(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend){
	IoTSockTrans *t = (IoTSockTrans *)pNetworkContext->tcpTransport;
	return t->transSend(pNetworkContext, pBuffer, bytesToSend);
}


/***
 * Read data from network socket. Non blocking returns 0 if no data
 * @param pNetworkContext - Used to locate the IoTSockTrans object to use
 * @param pBuffer - Buffer to read into
 * @param bytesToRecv - Maximum number of bytes to read
 * @return number of bytes read. May be 0 as non blocking
 * Negative number indicates error
 */
int32_t IoTSockTrans::staticRead(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv){
	IoTSockTrans *t = (IoTSockTrans *)pNetworkContext->tcpTransport;
	return t->transRead(pNetworkContext, pBuffer, bytesToRecv);
}

/***
 * Connect to remote TCP Socket
 * @param host - Host address
 * @param port - Port number
 * @return true on success
 */
bool IoTSockTrans::transConnect(const char * host, uint16_t port){

	strcpy(xHostName, host);
	xPort = port;
	int retval = 0;
	int32_t af;
	uint32_t ipLen = 4;
	uint8_t targetIP[4] ;

	af = IOT_SOCKET_AF_INET;

	retval = iotSocketGetHostByName (xHostName,af, targetIP, &ipLen);
	if (retval != 0){
		printf("DNS Request failed \r\n");
	}

	xSock = iotSocketCreate (af, IOT_SOCKET_SOCK_STREAM, IOT_SOCKET_IPPROTO_TCP);
	if (xSock < 0){
		LogError(("ERROR opening socket\n"));
		return false;
	}

	int res = iotSocketConnect(xSock, targetIP, ipLen, xPort);
	if (res < 0){
		LogError(("ERROR connecting 0x%X to %s port %d\n",res, xHostName, xPort));
		return false;
	}


	uint32_t nonblocking = 1U;
	iotSocketSetOpt (xSock, IOT_SOCKET_IO_FIONBIO, &nonblocking, sizeof(nonblocking));

	LogInfo(("Connect success\n"));
	return true;
}


/***
 * Close the socket
 * @return true on success
 */
bool IoTSockTrans::transClose(){
	iotSocketClose(xSock);
	return true;
}


/***
 * Print the buffer in hex and plain text for debugging
 */
void IoTSockTrans::debugPrintBuffer(const char *title, const void * pBuffer, size_t bytes){
	size_t count =0;
	size_t lineEnd=0;
	const uint8_t *pBuf = (uint8_t *)pBuffer;

	printf("DEBUG: %s of size %d\n", title, bytes);

	while (count < bytes){
		lineEnd = count + DEBUG_LINE;
		if (lineEnd > bytes){
			lineEnd = bytes;
		}

		//Print HEX DUMP
		for (size_t i=count; i < lineEnd; i++){
			if (pBuf[i] <= 0x0F){
				printf("0%X ", pBuf[i]);
			} else {
				printf("%X ", pBuf[i]);
			}
		}

		//Pad for short lines
		size_t pad = (DEBUG_LINE - (lineEnd - count)) * 3;
		for (size_t i=0; i < pad; i++){
			printf(" ");
		}

		//Print Plain Text
		for (size_t i=count; i < lineEnd; i++){
			if ((pBuf[i] >= 0x20) && (pBuf[i] <= 0x7e)){
				printf("%c", pBuf[i]);
			} else {
				printf(".");
			}
		}

		printf("\n");

		count = lineEnd;

	}
}


