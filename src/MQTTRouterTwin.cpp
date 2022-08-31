/*
 * MQTTRouterTwin.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: jondurrant
 */

#include "MQTTRouterTwin.h"
#include "MQTTTopicHelper.h"

MQTTRouterTwin::MQTTRouterTwin() {
	// TODO Auto-generated constructor stub

}

MQTTRouterTwin::~MQTTRouterTwin() {
	if (pSetTopic != NULL){
		vPortFree(pSetTopic);
		pSetTopic = NULL;
	}
	if (pGetTopic != NULL){
		vPortFree(pGetTopic);
		pGetTopic = NULL;
	}
}


MQTTRouterTwin::MQTTRouterTwin(const char * id, MQTTInterface *mi){
	init(id, mi);
}

/***
 * Initialise the object give the Id and MQTT Interface
 * @param id = string ID of the Thing
 * @param mi = MQTT Interface
 */
void MQTTRouterTwin::init(const char * id, MQTTInterface *mi){
	MQTTRouterPing::init(id, mi);

	if (pSetTopic == NULL){
		pSetTopic = (char *)pvPortMalloc(
				MQTTTopicHelper::lenThingSet(id)
				);
		if (pSetTopic != NULL){
			MQTTTopicHelper::getThingSet(pSetTopic, id);
		} else {
			LogError( ("Unable to allocate topic") );
		}
	}

	if (pGetTopic == NULL){
		pGetTopic = (char *)pvPortMalloc(
				MQTTTopicHelper::lenThingGet(id)
				);
		if (pGetTopic != NULL){
			MQTTTopicHelper::getThingGet(pGetTopic, id);
		} else {
			LogError( ("Unable to allocate topic") );
		}
	}
}

/***
 * Use the interface to setup all the subscriptions
 * @param interface
 */
void MQTTRouterTwin::subscribe(MQTTInterface *interface){
	MQTTRouterPing::subscribe(interface);

	interface->subToTopic(pSetTopic, 1);
	interface->subToTopic(pGetTopic, 1);

}

/***
 * Route the message the appropriate part of the application
 * @param topic
 * @param topicLen
 * @param payload
 * @param payloadLen
 * @param interface
 */
void MQTTRouterTwin::route(const char *topic, size_t topicLen, const void * payload,
		size_t payloadLen, MQTTInterface *interface){
	MQTTRouterPing::route(topic, topicLen, payload, payloadLen, interface);

	if (pTwin == NULL){
		LogError(("Twin not defined"));
		return;
	}

	if (strlen(pSetTopic) == topicLen){
		if (memcmp(topic, pSetTopic, topicLen)==0){
			LogDebug(("STATE SET"));
			pTwin->addMessage((char *)payload, payloadLen);
		}
	}
	if (strlen(pGetTopic) == topicLen){
		if (memcmp(topic, pGetTopic, topicLen)==0){
			LogDebug(("STATE GET"));
			pTwin->addMessage(MQTT_STATE_PAYLOAD_GET,
					strlen(MQTT_STATE_PAYLOAD_GET)
					);
		}
	}
}

/***
 * set the TwinTasl
 * @param state
 */
void MQTTRouterTwin::setTwin(TwinTask *twin){
	pTwin = twin;
	pTwin->setMQTTInterface(pInterface);
}

