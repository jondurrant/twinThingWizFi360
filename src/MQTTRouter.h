/*
 * MQTTRouter.h
 *
 * Abstract router interface.
 * Defines interface for MQTTTask to get messages handled by application
 *
 *  Created on: 31 Oct 2021
 *      Author: jondurrant
 */

#ifndef MQTTROUTER_H_
#define MQTTROUTER_H_


#include "MQTTInterface.h"
#include "MQTTPingTask.h"

class MQTTRouter {
public:
	MQTTRouter();
	virtual ~MQTTRouter();

	/***
	 * Initialise the object give the Id and MQTT Interface
	 * @param id = string ID of the Thing
	 * @param mi = MQTT Interface
	 */
	virtual void init(const char * id, MQTTInterface *mi);


	/***
	 * Use interface to set subscriptions for the router
	 * @param interface
	 */
	 virtual void subscribe(MQTTInterface *interface)=0;


	 /***
	  * Route a message. Handle the message and connect to wider application
	  * @param topic - non zero terminated string
	  * @param topicLen - length of topic
	  * @param payload - memory structure of payload
	  * @param payloadLen - payload length
	  * @param interface - MQTT interface to use for any response publication
	  */
	 virtual void route(const char *topic, size_t topicLen, const void * payload, size_t payloadLen, MQTTInterface *interface)=0;

protected:
	 MQTTInterface *pInterface = NULL;
	 const char * id = NULL;


};

#endif /* MQTTROUTER_H_ */
