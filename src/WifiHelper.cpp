/*
 * WifiAgent.cpp
 *
 *  Created on: 24 Nov 2021
 *      Author: jondurrant
 */

#include "WifiHelper.h"
#include "MQTTConfig.h"

#include "lwesp/lwesp.h"
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include "FreeRTOS.h"
#include "task.h"

lwesp_datetime_t WifiHelper::dateTime;

WifiHelper::WifiHelper() {
	// TODO Auto-generated constructor stub

}



WifiHelper::~WifiHelper() {
	// TODO Auto-generated destructor stub
}


bool WifiHelper::connectToAp(const char * sid, const char *passwd){
	lwespr_t eres;
	char ipStr[16];

	/* Initialize ESP with default callback function */
	//LogInfo(("Initializing LwESP"));
	printf("Initializing LwESP\r\n");
	WifiHelper::setupGPIO();

	if (lwesp_init(NULL, 1) != lwespOK) {
		LogInfo(("First inilialize failed, h/w resetting"));
		WifiHelper::resetESP01();
		if (lwesp_init(NULL, 1) != lwespOK) {
			LogError(("Cannot initialize LwESP!"));
			return false;
		}
	}

	LogInfo(("LwESP initialized!"));


	if (lwesp_sta_has_ip()) {
		LogDebug(("Already connected"));

	} else {
		if (lwesp_sta_join(sid, passwd, NULL, NULL, NULL, 1) == lwespOK) {

			 LogInfo(("Connected to %s network!", sid));

			 //enable autorejoin
			LogDebug(("Autojoin %d", lwesp_sta_autojoin(1, NULL, NULL, 1) ));

		 } else {
			 LogError(("Connection error: %d", (int)eres));
			 return false;
		 }
	}
	return true;

}


/***
 * Get IP address of unit
 * @param ip - output uint8_t[4]
 */
bool WifiHelper::getIPAddress(uint8_t *ip){
	lwesp_ip_t lwip;
	uint8_t is_dhcp;

	if (!lwesp_sta_has_ip() == 1)
		return false;

	lwespr_t res = lwesp_sta_copy_ip(&lwip, NULL, NULL, &is_dhcp);

	memcpy(ip, lwip.addr.ip4.addr, 4);
	return (res == lwespOK);
}

/***
 * Get IP address of unit
 * @param ips - output char * up to 16 chars
 * @return - true if IP addres assigned
 */
bool WifiHelper::getIPAddressStr(char *ips){
	lwesp_ip_t lwip;
		uint8_t is_dhcp;

	if (!lwesp_sta_has_ip() == 1)
		return false;

	lwespr_t res = lwesp_sta_copy_ip(&lwip, NULL, NULL, &is_dhcp);

	if (res != lwespOK)
		return false;

	sprintf(ips, "%d.%d.%d.%d",
			lwip.addr.ip4.addr[0],
			lwip.addr.ip4.addr[1],
			lwip.addr.ip4.addr[2],
			lwip.addr.ip4.addr[3]
			);
	return true;

}


void WifiHelper::setupGPIO(){
	gpio_init(ESP01_RST_PIN);
	gpio_set_dir(ESP01_RST_PIN, GPIO_OUT);
	gpio_put(ESP01_RST_PIN, 1);
	vTaskDelay(ESP01_RST_DELAY/2);
}

void WifiHelper::resetESP01(){
	gpio_put(ESP01_RST_PIN, 0);
	vTaskDelay(ESP01_RST_DELAY/2);
	gpio_put(ESP01_RST_PIN, 1);
	vTaskDelay(ESP01_RST_DELAY/2);
}

bool WifiHelper::getMACAddressStr(char *macStr){
	lwespr_t res;
	lwesp_mac_t mac;
	res = lwesp_sta_getmac(&mac, NULL, NULL, 1);
	if (res == lwespOK){
		for (uint8_t i=0; i < 6; i++ ){
			lwesp_u8_to_hex_str(mac.mac[i], &macStr[i*2], 2);
		}
		macStr[13]=0;
		return true;
	}
	return false;
}



bool WifiHelper::syncRTCwithSNTP(){
	lwespr_t res;
	res = lwesp_sntp_set_config(1, ESP01_SNTP_TIMEZONE,
			ESP01_SNTP_LOCAL_HOST,
			"80.86.38.193",
			"79.135.97.79",
			WifiHelper::sntpSetupCB, NULL, 0);
	if (res != lwespOK){
		LogError(("sntp error %d", res));
	}
	return (res == lwespOK);
}

void WifiHelper::sntpSetupCB(lwespr_t resp, void * args){
	lwespr_t res;

	res = lwesp_sntp_gettime(&WifiHelper::dateTime,  WifiHelper::syncRTCCB, NULL, 0);
	if (res != lwespOK) {
		 LogError(("SNTP Gettime error %d", res));
	}
}

void WifiHelper::syncRTCCB(lwespr_t resp, void * args){
	if (resp == lwespOK) {

		 if (WifiHelper::dateTime.year < 2020){
			 LogDebug(("SNTP Year too low, rerequesting"));
			 lwesp_sntp_gettime(&WifiHelper::dateTime,  WifiHelper::syncRTCCB, NULL, 0);
		 } else {
			 rtc_init();
			 datetime_t t = {
			             .year  = (int16_t)WifiHelper::dateTime.year,
			             .month = (int8_t) WifiHelper::dateTime.month,
			             .day   = (int8_t) WifiHelper::dateTime.date,
			             .dotw  = ((int8_t) WifiHelper::dateTime.day) - (int8_t)1, // 0 is Sunday, so 5 is Friday
			             .hour  = (int8_t) WifiHelper::dateTime.hours,
			             .min   = (int8_t) WifiHelper::dateTime.minutes,
			             .sec   = (int8_t) WifiHelper::dateTime.seconds
			     };
			 rtc_set_datetime(&t);

			 LogInfo(("RTC Set %d-%d-%d %d:%d:%d",
					 WifiHelper::dateTime.year,
					 WifiHelper::dateTime.month,
					 WifiHelper::dateTime.date,
					 WifiHelper::dateTime.hours,
					 WifiHelper::dateTime.minutes,
					 WifiHelper::dateTime.seconds
					 ));
		 }

	} else {
		LogError(("lwesp_sntp_gettime failed %d",resp));
	}

}



bool WifiHelper::isJoined(){
	return (lwesp_sta_is_joined() == 1);
}



bool WifiHelper::autoJoinOrConfig(){
	lwespr_t eres;
	char ipStr[16];

	/* Initialize ESP with default callback function */
	//LogInfo(("Initializing LwESP"));
	printf("Initializing LwESP\r\n");
	WifiHelper::setupGPIO();

	if (lwesp_init(NULL, 1) != lwespOK) {
		LogInfo(("First inilialize failed, h/w resetting"));
		WifiHelper::resetESP01();
		if (lwesp_init(NULL, 1) != lwespOK) {
			LogError(("Cannot initialize LwESP!"));
			return false;
		}
	}

	LogInfo(("LwESP initialized!"));


	LogDebug(("Autojoin %d", lwesp_sta_autojoin(1, NULL, NULL, 1) ));

	while (!WifiHelper::isJoined()){
		vTaskDelay(5000);
		LogInfo(("Wait for autojoin"));
	}

	return true;

}
