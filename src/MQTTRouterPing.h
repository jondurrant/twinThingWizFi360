/*
 * MQTTRouterPing.h
 *
 * Example router responding to Ping message with a pong response
 *
 *  Created on: 31 Oct 2021
 *      Author: jondurrant
 */

#ifndef MQTTROUTERPING_H_
#define MQTTROUTERPING_H_

#include "MQTTConfig.h"
#include "MQTTRouter.h"
#include "MQTTPingTask.h"
#include "MQTTInterface.h"


class MQTTRouterPing : public MQTTRouter{
public:
	/***
	 * Constructor
	 */
	MQTTRouterPing();

	/***
	 * Constructor providing Id for the Thing and MQTT Interface
	 * @param id - string ID of the thing
	 * @param mi - MQTT Interface
	 */
	MQTTRouterPing(const char * id, MQTTInterface *mi);

	/***
	 * Destructor
	 */
	virtual ~MQTTRouterPing();

	/***
	 * Initialise the object give the Id and MQTT Interface
	 * @param id = string ID of the Thing
	 * @param mi = MQTT Interface
	 */
	virtual void init(const char * id, MQTTInterface *mi);

	/***
	 * Set Task to use for action
	 * @param p
	 */
	virtual void setPingTask(MQTTPingTask *p);

	/***
	 * Use the interface to setup all the subscriptions
	 * @param interface
	 */
	virtual void subscribe(MQTTInterface *interface);

	/***
	 * Route the message the appropriate part of the application
	 * @param topic
	 * @param topicLen
	 * @param payload
	 * @param payloadLen
	 * @param interface
	 */
	virtual void route(const char *topic, size_t topicLen, const void * payload, size_t payloadLen, MQTTInterface *interface);

private:

	MQTTPingTask *pingTask = NULL;

	char * pingTopic = NULL;
	char * pongTopic = NULL;
	char * allPingTopic = NULL;

};

#endif /* MQTTROUTEREXP_H_ */
