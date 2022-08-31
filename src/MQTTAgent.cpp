/*
 * MQTTAgent.cpp
 *
 *  Created on: 15 Nov 2021
 *      Author: jondurrant
 */

#include <stdlib.h>

#include "MQTTAgent.h"
#include "MQTTTopicHelper.h"

const char * MQTTAgent::WILLPAYLOAD = "{\"online\":0}";
const char * MQTTAgent::ONLINEPAYLOAD = "{\"online\":1}";


/***
	 * Constructor
	 * @param rxBufSize - size of rx buffer to create
	 * @param txBufSize - size of tx buffer to create
	 */
MQTTAgent::MQTTAgent(size_t rxBufSize, size_t txBufSize) {
	xRxBufSize = rxBufSize;
	xTxBufSize = txBufSize;
}

/***
 * Destructor
 */
MQTTAgent::~MQTTAgent() {
	if (pMQTTClient != NULL){
		lwesp_mqtt_client_api_delete(pMQTTClient);
	}

	if (willTopic != NULL){
		vPortFree(willTopic);
		willTopic = NULL;
	}

	if (onlineTopic != NULL){
		vPortFree(onlineTopic);
		onlineTopic = NULL;
	}

	if (keepAliveTopic != NULL){
		vPortFree(keepAliveTopic);
		keepAliveTopic = NULL;
	}

}

/***
 * Stop task
 * @return
 */
void MQTTAgent::stop(){
	if (xHandle != NULL){
		vTaskDelete(  xHandle );
		xHandle = NULL;
	}

	if (pMQTTClient != NULL){
		lwesp_mqtt_client_api_delete(pMQTTClient);
		pMQTTClient = NULL;
	}
}


/***
* Initialise the object
* @return
*/
bool MQTTAgent::init(){
	pMQTTClient = lwesp_mqtt_client_api_new(xRxBufSize, xTxBufSize);
	if (pMQTTClient == NULL) {
		LogError( ("MQTTAgent::init mqtt  failed\n") );
		return false;
	} else {
		LogDebug( ("MQTTAgent::init complete\n") );
	}

	return true;
}


/***
 * Set credentials
 * @param user - string pointer. Not copied so pointer must remain valid
 * @param passwd - string pointer. Not copied so pointer must remain valid
 * @param id - string pointer. Not copied so pointer must remain valid. I
 * f not provide ID will be user
 * @return lwespOK if succeeds
 */
void MQTTAgent::credentials(const char * user, const char * passwd, const char * id){
	this->user = user;
	this->passwd = passwd;
	if (id != NULL){
		this->id = id;
	} else {
		this->id = user;
	}

	if (willTopic == NULL){
		willTopic = (char *)pvPortMalloc( MQTTTopicHelper::lenLifeCycleTopic(this->id, MQTT_TOPIC_LIFECYCLE_OFFLINE));
		if (willTopic != NULL){
			MQTTTopicHelper::genLifeCycleTopic(willTopic, this->id, MQTT_TOPIC_LIFECYCLE_OFFLINE);
		} else {
			LogError( ("Unable to allocate LC topic") );
		}
	}

	if (onlineTopic == NULL){
		onlineTopic = (char *)pvPortMalloc( MQTTTopicHelper::lenLifeCycleTopic(this->id, MQTT_TOPIC_LIFECYCLE_ONLINE));
		if (onlineTopic != NULL){
			MQTTTopicHelper::genLifeCycleTopic(onlineTopic, this->id, MQTT_TOPIC_LIFECYCLE_ONLINE);
		} else {
			LogError( ("Unable to allocate LC topic") );
		}
	}

	if (keepAliveTopic == NULL){
		keepAliveTopic = (char *)pvPortMalloc( MQTTTopicHelper::lenLifeCycleTopic(this->id, MQTT_TOPIC_LIFECYCLE_KEEP_ALIVE));
		if (keepAliveTopic != NULL){
			MQTTTopicHelper::genLifeCycleTopic(keepAliveTopic, this->id, MQTT_TOPIC_LIFECYCLE_KEEP_ALIVE);
		} else {
			LogError( ("Unable to allocate LC topic") );
		}
	}
	//printf("MQTT Credentials Id=%s, usr=%s, pwd=%s\n", this->id, this->user, this->passwd);
}

/***
 * Connect to mqtt server
 * @param target - hostname or ip address, Not copied so pointer must remain valid
 * @param port - port number
 * @param ssl - unused
 * @return
 */
bool MQTTAgent::connect(char * target, lwesp_port_t  port, bool recon, bool ssl){
	this->target = target;
	this->port = port;
	this->recon = recon;
	this->ssl = ssl;
	setConnState(MQTTConn);
	return true;
}



/***
*  create the vtask, will get picked up by scheduler
*
*  */
void MQTTAgent::start(UBaseType_t priority){
	if (init() ){
		xTaskCreate(
			MQTTAgent::vTask,
			"MQTTAgent",
			512,
			( void * ) this,
			priority,
			&xHandle
		);
	}
}

/***
 * Internal function used by FreeRTOS to run the task
 * @param pvParameters
 */
 void MQTTAgent::vTask( void * pvParameters ){
	 MQTTAgent *task = (MQTTAgent *) pvParameters;
	 if (task->init()){
		 task->run();
	 }
 }

/***
* Run loop for the task
*/
 void MQTTAgent::run(){
	 LogDebug( ("MQTTAgent run\n") );


	 for(;;){

		 switch(connState){
		 case Offline: {
			 break;
		 }
		 case MQTTConn: {
			 mqttConn();
			 break;
		 }
		 case MQTTConned: {
			 mqttSub();
			 setConnState(Online);
			 pubToTopic(onlineTopic, ONLINEPAYLOAD, strlen(ONLINEPAYLOAD), 1);

			 break;
		 }
		 case MQTTRecon: {
			 vTaskDelay(MQTT_RECON_DELAY);
			 setConnState(MQTTConn);
			 break;
		 }
		 case Online: {
			 mqttRec();

			 break;
		 }
		 default:{

		 }

		 };

		 taskYIELD();
	 }

 }




 /***
* Returns the id of the client
* @return
*/
const char * MQTTAgent::getId(){
	return id;
}

/***
* Publish message to topic
* @param topic - zero terminated string. Copied by function
* @param payload - payload as pointer to memory block
* @param payloadLen - length of memory block
*/
bool MQTTAgent::pubToTopic(const char * topic, const void * payload,
		size_t payloadLen, const uint8_t QoS){

	lwespr_t status;
	lwesp_mqtt_qos_t q;
	switch (QoS){
	case 0:{
		q = LWESP_MQTT_QOS_AT_MOST_ONCE;
		break;
	}
	case 1:{
		q = LWESP_MQTT_QOS_AT_LEAST_ONCE;
		break;
	}
	case 2:{
		q = LWESP_MQTT_QOS_EXACTLY_ONCE;
		break;
	}
	default:{
		q = LWESP_MQTT_QOS_AT_MOST_ONCE;
		break;
	}
	}

	if (connState == Online){
		LogDebug( ("Publish to: %s \n", topic ));

		if (pObserver != NULL){
			pObserver->MQTTSend();
		}
		status = lwesp_mqtt_client_api_publish(pMQTTClient, topic,
				payload, payloadLen, q, false);
		return (status == lwespOK);
	} else {
		return false;
	}

}

/***
* Close connection
*/
void MQTTAgent::close(){
	setConnState(Offline);
	lwesp_mqtt_client_api_close( pMQTTClient);
}

/***
* Route a message to the router object
* @param topic - non zero terminated string
* @param topicLen - topic length
* @param payload - raw memory
* @param payloadLen - payload length
*/
void MQTTAgent::route(const char * topic, size_t topicLen, const void * payload, size_t payloadLen){
	if (pObserver != NULL){
		pObserver->MQTTRecv();
	}

	if (pRouter != NULL){
		pRouter->route(topic, topicLen, payload, payloadLen, this);
	}
}


/***
* Get the router object handling all received messages
* @return
*/
MQTTRouter* MQTTAgent::getRouter()  {
	return pRouter;
}

/***
* Set the rotuer object
* @param pRouter
*/
void MQTTAgent::setRouter( MQTTRouter *pRouter) {
	this->pRouter = pRouter;
}

/***
* Connect to server
* @return
*/
bool MQTTAgent::mqttConn(){
	lwesp_mqtt_conn_status_t conn_status;


	xMqttClientInfo.id = id;
	xMqttClientInfo.user = user;
	xMqttClientInfo.pass = passwd;
	xMqttClientInfo.keep_alive = MQTT_KEEP_ALIVE;
	xMqttClientInfo.will_topic = willTopic;
	xMqttClientInfo.will_message = WILLPAYLOAD;
	xMqttClientInfo.will_qos = LWESP_MQTT_QOS_AT_LEAST_ONCE;
#ifdef LWESPFORK
	xMqttClientInfo.ssl = this->ssl;
	LogInfo(("CONNECTING with SSL set to %d", this-ssl));
#endif

	conn_status = lwesp_mqtt_client_api_connect(pMQTTClient,
			target, port, &xMqttClientInfo);
	if (conn_status == LWESP_MQTT_CONN_STATUS_ACCEPTED) {
		setConnState(MQTTConned);
		LogDebug( ("Connected and accepted!\r\n") );
	} else {
		setConnState(Offline);
		//printf("MQTT Connect Failed: %d\r\n", (int)conn_status);
		LogError( ("MQTT Connect Failed %d\n", (int)conn_status ));
		if (recon){
			setConnState(MQTTRecon);
		}
		return false;
	}
	return true;
}


/***
* Subscribe to a topic, mesg will be sent to router object
* @param topic
* @param QoS
* @return
*/
bool MQTTAgent::subToTopic(const char * topic, const uint8_t QoS){
	LogDebug( ("Subscribe tp %s\n", topic) );

	lwespr_t status;
	lwesp_mqtt_qos_t q;
	switch (QoS){
	case 0:{
		q = LWESP_MQTT_QOS_AT_MOST_ONCE;
		break;
	}
	case 1:{
		q = LWESP_MQTT_QOS_AT_LEAST_ONCE;
		break;
	}
	case 2:{
		q = LWESP_MQTT_QOS_EXACTLY_ONCE;
		break;
	}
	default:{
		q = LWESP_MQTT_QOS_AT_MOST_ONCE;
		break;
	}
	}
	status = lwesp_mqtt_client_api_subscribe( pMQTTClient,
			topic, q);
	return (status == lwespOK);
}

/***
* Subscribe step on connection
* @return
*/
bool MQTTAgent::mqttSub(){

	if (pRouter != NULL){
		pRouter->subscribe(this);
		return true;
	}
	return false;
}

/***
* Handle Rec of a messahe
* @return
*/
bool MQTTAgent::mqttRec(){
	lwespr_t res;
	lwesp_mqtt_client_api_buf_p buf;
	res = lwesp_mqtt_client_api_receive(pMQTTClient, &buf, 5000);
	if (res == lwespOK) {
		if (buf != NULL) {
			//printf("Topic: %s, payload: %s\r\n", buf->topic, buf->payload);
			route(buf->topic, buf->topic_len,
					buf->payload, buf->payload_len );
			lwesp_mqtt_client_api_buf_free(buf);
			buf = NULL;
		}
	} else if (res == lwespCLOSED) {
		LogWarn( ("MQTT connection closed!\r\n") );
		if (recon){
			LogDebug( ("Reconnect") );
			setConnState(MQTTRecon);
		} else {
			setConnState(Offline);
		}
		return false;
	} else if (res == lwespTIMEOUT) {
		LogDebug( ("Timeout on MQTT receive function. Manually publishing.\r\n") );
		pubToTopic(keepAliveTopic, ONLINEPAYLOAD, strlen(ONLINEPAYLOAD), 1);
	}
	return true;
}

/***
 * Set the connection state variable
 * @param s
 */
void MQTTAgent::setConnState(MQTTState s){
	connState = s;

	if (pObserver != NULL){
		switch(connState){
		case Offline:{
			pObserver->MQTTOffline();
			break;
		}
		case Online:{
			pObserver->MQTTOnline();
			break;
		}
		default:{
			;
		}
		}
	}
}

/***
 * Set a single observer to get call back on state changes
 * @param obs
 */
void MQTTAgent::setObserver(MQTTAgentObserver *obs){
	pObserver = obs;
}


