/*
 * StateTask.cpp
 *
 *  Created on: 22 Nov 2021
 *      Author: jondurrant
 */

#include "TwinTask.h"
#include "MQTTTopicHelper.h"
#include "TwinProtocol.h"
#include <cstring>

TwinTask::TwinTask() {
	// TODO Auto-generated constructor stub

}

TwinTask::~TwinTask() {
	stop();
	if (updateTopic != NULL){
		vPortFree(updateTopic);
		updateTopic = NULL;
	}
}


void TwinTask::setStateObject(State *state){
	pState = state;
	pState->attach(this);
}

State *TwinTask::getStateObject(){
	return pState;
}


void TwinTask::setMQTTInterface(MQTTInterface *mi){
	mqttInterface = mi;
	if (updateTopic == NULL){
		updateTopic = (char *)pvPortMalloc(
				MQTTTopicHelper::lenThingUpdate(mi->getId())
				);
		if (updateTopic != NULL){
			MQTTTopicHelper::getThingUpdate(updateTopic, mi->getId());
		} else {
			LogError( ("Allocated failed") );
		}
	}
}

bool TwinTask::addMessage(const char * msg, size_t msgLen){
	if (xMessageBuffer != NULL){
		size_t res = xMessageBufferSend(xMessageBuffer,
					msg, msgLen, 0 );
			if (res != msgLen){
				LogError( ("addMessage failed. Msg len:%d  Buf:%d Msg:%s\n", msgLen, xMessageBufferSpacesAvailable(xMessageBuffer), msg ));
				return false;
			}
	} else {
		LogError( ("No Message Buf") );
		return false;
	}
	//LogDebug(("addMessage succeed(%d):%s\n", msgLen, msg));
	return true;
}

/***
 *  create the vtask, will get picked up by scheduler
 *
 *  */
void TwinTask::start(UBaseType_t priority){
	xMessageBuffer = xMessageBufferCreate( STATE_MSG_BUF_LEN );
	if (xMessageBuffer == NULL){
		LogError( ("Create buf failed") );
		return;
	}
	if (xMessageBuffer != NULL){
		xTaskCreate(
			TwinTask::vTask,
			"MQTTstate",
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
void TwinTask::stop(){
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
void TwinTask::vTask( void * pvParameters ){
	TwinTask *task = (TwinTask *) pvParameters;
	task->run();
}

/***
 * Internal function to run the task from within the object
 */
void TwinTask::run(){

	for (;;){
		if (xMessageBufferIsEmpty(xMessageBuffer) == pdTRUE){
			taskYIELD();
		} else {
			vTaskDelay( 20 );
			size_t size = xMessageBufferReceive(xMessageBuffer,
								xMsg, STATE_MAX_MSG_LEN, 0);
			if (size > 0){
				if (pState != NULL){
					processMsg(xMsg);
				}
			}
		}
	}
}


/***
* Process a json message received
* @param str
*/
void TwinTask::processMsg(char *str){
	json_t const* json = json_create( str, jsonBuf, jsonBufLen );
	if ( !json ) {
		LogError( ("json create. %s",str) );
		return;
	}

	json_t const* delta = json_getProperty(json, TWINDELTA);
	if (!delta){
		delta = json_getProperty(json, TWINSTATE);
	}

	if (delta){
		pState->startTransaction();
		pState->updateFromJson(delta);
		pState->commitTransaction();
	}

	//GET Processing
	if (!delta){
		delta = json_getProperty(json, MQTT_STATE_TOPIC_GET);
		if (delta){
			LogDebug(("Handling Get"));
			pState->state(xMsg, STATE_MAX_MSG_LEN);
			mqttInterface->pubToTopic(updateTopic, xMsg, strlen(xMsg), 1);
		}
	}

	processJson(json);

	if (pState != NULL){
		if (pState->isDirty()){
			LogDebug(("State is dirty so sending delta"));
			pState->delta(xMsg, STATE_MAX_MSG_LEN);
			mqttInterface->pubToTopic(updateTopic, xMsg, strlen(xMsg), 1);
		}
		else {
			LogDebug(("State clean after processing json"));
		}
	}
}


/***
* Process a json message received
* Extend this for subclass processing
* @param json
*/
void TwinTask::processJson(json_t const* json){
	//NOP
}


/***
 * Notification of a change of a state item with the State object.
 * @param dirtyCode - Representation of item changed within state. Used to pull back delta
 */
void TwinTask::notifyState(uint16_t dirtyCode){
	if (mqttInterface != NULL){
		unsigned int i;

		i = pState->delta(xMsg, STATE_MAX_MSG_LEN, dirtyCode);
		if (i == 0){
			LogError(("Buf overrun"));
		}
		mqttInterface->pubToTopic(updateTopic, xMsg, strlen(xMsg));
	}
}
