/*
 * MQTTRouter.cpp
 *
 *  Created on: 31 Oct 2021
 *      Author: jondurrant
 */

#include "MQTTRouter.h"

MQTTRouter::MQTTRouter() {
	// TODO Auto-generated constructor stub

}

MQTTRouter::~MQTTRouter() {
	// TODO Auto-generated destructor stub
}


/***
 * Initialise the object give the Id and MQTT Interface
 * @param id = string ID of the Thing
 * @param mi = MQTT Interface
 */
void MQTTRouter::init(const char * id, MQTTInterface *mi){
	pInterface = mi;
	this->id = id;

}
