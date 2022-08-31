/*
 * MQTTPingTask.cpp
 *
 *  Created on: 14 Nov 2021
 *      Author: jondurrant
 */

#include "MQTTPingTask.h"

/***
* Constructor
*/
MQTTPingTask::MQTTPingTask() {
	// NOP

}

/***
 * Destructor
 */
MQTTPingTask::~MQTTPingTask() {
	stop();
}

/***
 * Set the MQTT Interface
 * @param mi - MQTTIntercace, an MQTTAgent object
 */
void MQTTPingTask::setInterface(MQTTInterface * mi){
	mqttInterface = mi;
}

/***
* Set the response topic
* @param topic = string
*/
void MQTTPingTask::setPongTopic(char * topic){
	pongTopic = topic;
}

/***
 *  create the vtask, will get picked up by scheduler
 *
 *  */
void MQTTPingTask::start(UBaseType_t priority){
	xMessageBuffer = xMessageBufferCreate( PINGBUFFER );
	if (xMessageBuffer == NULL){
		LogError( ("No MQTTPingTask PINGBUFFER") );
		return;
	}
	if (xMessageBuffer != NULL){
		xTaskCreate(
			MQTTPingTask::vTask,
			"MQTTping",
			512,
			( void * ) this,
			priority,
			&xHandle
		);
	}
}

/***
* Stop the vtask
*/
void MQTTPingTask::stop(){
	if (xHandle != NULL){
		vTaskDelete(  xHandle );
		xHandle = NULL;
	}

	if (xMessageBuffer != NULL){
		vMessageBufferDelete(xMessageBuffer);
		xMessageBuffer = NULL;
	}
}

/***
 * Internal function used by FreeRTOS to run the task
 * @param pvParameters
 */
void MQTTPingTask::vTask( void * pvParameters ){
	MQTTPingTask *task = (MQTTPingTask *) pvParameters;
	task->run();
}

/***
* Add a ping request to the queue
* @param payload
* @param payloadLen
* @return
*/
bool MQTTPingTask::addPing(const void * payload, size_t payloadLen){
	size_t res = xMessageBufferSend(xMessageBuffer,
			payload, payloadLen, 0 );
	if (res != payloadLen){
		LogError( ("MQTTPingTask::addPing failed\n") );
		return false;
	}
	return true;
}


/***
* Internal function to run the task from within the object
*/
void MQTTPingTask::run(){
	char payload[MAXPINGPAYLOAD];
	for (;;){
		if (xMessageBufferIsEmpty(xMessageBuffer) == pdTRUE){
			taskYIELD();
		} else {
			vTaskDelay( 20 );
			size_t size = xMessageBufferReceive(xMessageBuffer,
								payload, MAXPINGPAYLOAD, 0);
			if (size > 0){
				if ((pongTopic != NULL) && (mqttInterface != NULL)){
					mqttInterface->pubToTopic(pongTopic, payload, size);
				} else {
					LogError( ("MQTTPingTask interface or topic not set") );
				}
			}
		}
	}

}
