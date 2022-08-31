/*
 * MQTTAgent.h
 *
 *  Created on: 15 Nov 2021
 *      Author: jondurrant
 */

#ifndef MQTTAGENT_H_
#define MQTTAGENT_H_

#include "FreeRTOS.h"
#include "lwesp/lwesp.h"
#include "lwesp/apps/lwesp_mqtt_client_api.h"

#include "MQTTConfig.h"
#include "MQTTInterface.h"
#include "MQTTRouter.h"
#include "MQTTAgentObserver.h"


#ifndef MQTT_RECON_DELAY
#define MQTT_RECON_DELAY 3000
#endif

#ifndef MQTT_KEEP_ALIVE
#define MQTT_KEEP_ALIVE 10
#endif


enum MQTTState {  Offline, MQTTConn, MQTTRecon, MQTTConned, Online};

class MQTTAgent: public MQTTInterface {
public:
	/***
	 * Constructor
	 * @param rxBufSize - size of rx buffer to create
	 * @param txBufSize - size of tx buffer to create
	 */
	MQTTAgent(size_t rxBufSize, size_t txBufSize);

	/***
	 * Destructor
	 */
	virtual ~MQTTAgent();

	/***
	 * Set credentials
	 * @param user - string pointer. Not copied so pointer must remain valid
	 * @param passwd - string pointer. Not copied so pointer must remain valid
	 * @param id - string pointer. Not copied so pointer must remain valid. I
	 * f not provide ID will be user
	 * @return lwespOK if succeeds
	 */
	void credentials(const char * user, const char * passwd, const char * id = NULL );

	/***
	 * Connect to mqtt server
	 * @param target - hostname or ip address, Not copied so pointer must remain valid
	 * @param port - port number
	 * @param recon - reconnect on disconnect
	 * @return
	 */
	 bool connect(char * target, lwesp_port_t  port, bool recon=false, bool ssl=false);

	 /***
	 *  create the vtask, will get picked up by scheduler
	 *
	 *  */
	void start(UBaseType_t priority = tskIDLE_PRIORITY);

	/***
	 * Stop task
	 * @return
	 */
	void stop();


	/***
	 * Returns the id of the client
	 * @return
	 */
	virtual const char * getId();

	/***
	 * Publish message to topic
	 * @param topic - zero terminated string. Copied by function
	 * @param payload - payload as pointer to memory block
	 * @param payloadLen - length of memory block
	 * @param QoS, QoS level of publish (0-2)
	 */
	virtual bool pubToTopic(const char * topic, const void * payload,
			size_t payloadLen, const uint8_t QoS=0);

	/***
	 * Close connection
	 */
	virtual void close();

	/***
	 * Route a message to the router object
	 * @param topic - non zero terminated string
	 * @param topicLen - topic length
	 * @param payload - raw memory
	 * @param payloadLen - payload length
	 */
	virtual void route(const char * topic, size_t topicLen, const void * payload, size_t payloadLen);

	/***
	 * Get the router object handling all received messages
	 * @return
	 */
	MQTTRouter* getRouter() ;

	/***
	 * Set the rotuer object
	 * @param pRouter
	 */
	void setRouter( MQTTRouter *pRouter = NULL);


	/***
	 * Subscribe to a topic, mesg will be sent to router object
	 * @param topic
	 * @param QoS
	 * @return
	 */
	virtual bool subToTopic(const char * topic, const uint8_t QoS=0);


	/***
	 * Set a single observer to get call back on state changes
	 * @param obs
	 */
	virtual void setObserver(MQTTAgentObserver *obs);

private:
	/***
	 * Task object running to manage MQTT interface
	 * @param pvParameters
	 */
	static void vTask( void * pvParameters );

	/***
	 * Initialise the object
	 * @return
	 */
	bool init();

	/***
	 * Run loop for the task
	 */
	void run();

	/***
	 * Connect to server
	 * @return
	 */
	bool mqttConn();

	/***
	 * Subscribe step on connection
	 * @return
	 */
	bool mqttSub();

	/***
	 * Handle Rec of a messahe
	 * @return
	 */
	bool mqttRec();

	/***
	 * Set the connection state variable
	 * @param s
	 */
	void setConnState(MQTTState s);

	//MQTT Server details
	const char * user;
	const char * passwd;
	const char * id;
	const char * target = NULL;
	lwesp_port_t port = 1883 ;
	bool recon = false;
	bool ssl = false;

	//Router object used for message processing
	MQTTRouter * pRouter = NULL;

	//The task
	TaskHandle_t xHandle = NULL;

	// Current state of the connection for run loop
	MQTTState connState = Offline;

	//Handling of the Will for connection
	//static const char * WILLTOPICFORMAT;
	char *willTopic = NULL;
	static const char * WILLPAYLOAD;
	char *onlineTopic = NULL;
	static const char * ONLINEPAYLOAD;
	char *keepAliveTopic = NULL;

	// MQTT Client handles and buffer sizes
	lwesp_mqtt_client_api_p pMQTTClient = NULL;
	lwesp_mqtt_client_info_t xMqttClientInfo;
	size_t xRxBufSize;
	size_t xTxBufSize;

	//Single Observer
	MQTTAgentObserver *pObserver = NULL;

};

#endif /* MQTTAGENT_H_ */
