/*
 * MQTTPingTask.h
 *
 * Task agent to queue ping request and publush pong responses
 *
 *  Created on: 14 Nov 2021
 *      Author: jondurrant
 */

#ifndef MQTTPINGTASK_H_
#define MQTTPINGTASK_H_

#include <stdlib.h>
#include <FreeRTOS.h>
#include <message_buffer.h>
#include <semphr.h>
#include <stdbool.h>
#include "MQTTConfig.h"
#include "MQTTInterface.h"

#define PINGBUFFER 250
#define MAXPINGPAYLOAD 20

class MQTTPingTask {
public:
	/***
	 * Constructor
	 */
	MQTTPingTask();

	/***
	 * Destructor
	 */
	virtual ~MQTTPingTask();

	/***
	 * Set the MQTT Interface
	 * @param mi - MQTTIntercace, an MQTTAgent object
	 */
	void setInterface(MQTTInterface * mi);

	/***
	 * Set the response topic
	 * @param topic = string
	 */
	void setPongTopic(char * topic);

	/***
	 *  create the vtask, will get picked up by scheduler
	 *
	 *  */
	void start(UBaseType_t priority = tskIDLE_PRIORITY);

	/***
	 * Stop the vtask
	 */
	void stop();

	/***
	 * Internal function used by FreeRTOS to run the task
	 * @param pvParameters
	 */
	static void vTask( void * pvParameters );

	/***
	 * Add a ping request to the queue
	 * @param payload
	 * @param payloadLen
	 * @return
	 */
	bool addPing(const void * payload, size_t payloadLen);

	/***
	 * Get the FreeRTOS task being used
	 * @return
	 */
	virtual TaskHandle_t getTask();

	/***
	 * Get high water for stack
	 * @return close to zero means overflow risk
	 */
	virtual unsigned int getStakHighWater();

protected:
	/***
	 * Internal function to run the task from within the object
	 */
	void run();

private:
	MessageBufferHandle_t xMessageBuffer = NULL;
	MQTTInterface *mqttInterface = NULL;
	TaskHandle_t xHandle = NULL;
	char * pongTopic = NULL;

};

#endif /* MQTTPINGTASK_H_ */
