/*
 * WifiAgent.cpp
 *
 *  Created on: 24 Nov 2021
 *      Author: jondurrant
 */

#include "WifiHelper.h"
#include "MQTTConfig.h"

#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <ctime>
#include "hardware/rtc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "iot_socket.h"
#include "Driver_WiFi.h"




#define SECURITY_TYPE   ARM_WIFI_SECURITY_WPA2

extern ARM_DRIVER_WIFI Driver_WiFi1;

WifiHelper::WifiHelper() {
	// TODO Auto-generated constructor stub

}



WifiHelper::~WifiHelper() {
	// TODO Auto-generated destructor stub
}


bool WifiHelper::connectToAp(const char * sid, const char *passwd){

	ARM_WIFI_CONFIG_t config;
	int32_t ret;
	uint8_t net_info[4];

	ret = Driver_WiFi1.Initialize  (NULL);
	if (ret != 0){
		LogError(("Driver_WiFix.Initialize  (NULL) = %d\r\n", ret));
		return false;
	}

	ret = Driver_WiFi1.PowerControl(ARM_POWER_FULL);
	if (ret != 0){
		LogError(("Driver_WiFix.PowerControl(ARM_POWER_FULL) = %d\r\n", ret));
		return false;
	}

	memset((void *)&config, 0, sizeof(config));

	config.ssid     = WIFI_SSID;
	config.pass     = WIFI_PASSWORD;
	config.security = SECURITY_TYPE;
	config.ch       = 0U;

	ret = Driver_WiFi1.Activate(0U, &config);
	if (ret != 0){
		LogError(("Driver_WiFix.Activate(0U, &config) = %d\r\n", ret));
	    return false;
	}


	if (!WifiHelper::isJoined()){
		LogError(("Wifi not connected"));
		return false;
	}


	uint32_t len = 4;
	Driver_WiFi1.GetOption(0, ARM_WIFI_IP, net_info, &len);
	LogInfo(("IP = %d.%d.%d.%d\r\n", net_info[0], net_info[1], net_info[2], net_info[3]));

	Driver_WiFi1.GetOption(0, ARM_WIFI_IP_SUBNET_MASK, net_info, &len);
	LogInfo(("MASK = %d.%d.%d.%d\r\n", net_info[0], net_info[1], net_info[2], net_info[3]));

	Driver_WiFi1.GetOption(0, ARM_WIFI_IP_GATEWAY, net_info, &len);
	LogInfo(("GATEWAY = %d.%d.%d.%d\r\n", net_info[0], net_info[1], net_info[2], net_info[3]));

	Driver_WiFi1.GetOption(0, ARM_WIFI_IP_DNS1, net_info, &len);
	LogInfo(("DNS1 = %d.%d.%d.%d\r\n", net_info[0], net_info[1], net_info[2], net_info[3]));

	Driver_WiFi1.GetOption(0, ARM_WIFI_IP_DNS2, net_info, &len);
	LogInfo(("DNS2 = %d.%d.%d.%d\r\n", net_info[0], net_info[1], net_info[2], net_info[3]));

	return true;

}


/***
 * Get IP address of unit
 * @param ip - output uint8_t[4]
 */
bool WifiHelper::getIPAddress(uint8_t *ip){
	uint32_t len = 4;
	Driver_WiFi1.GetOption(0, ARM_WIFI_IP, ip, &len);
	return true;
}

/***
 * Get IP address of unit
 * @param ips - output char * up to 16 chars
 * @return - true if IP addres assigned
 */
bool WifiHelper::getIPAddressStr(char *ips){
	uint8_t ip[4];
	if (getIPAddress(ip)){

		sprintf(ips, "%d.%d.%d.%d",
				ip[0],
				ip[1],
				ip[2],
				ip[3]
				);
		return true;
	}

	return false;

}

bool WifiHelper::getMACAddressStr(char *macStr){
	uint32_t len = 6;
	uint8_t mac[6];
	Driver_WiFi1.GetOption(0, ARM_WIFI_MAC, mac, &len);

	for (int i = 0; i < len; i++){
		if (mac[i] < 16){
			sprintf(&macStr[i*2],"0%X", mac[i]);
		} else {
			sprintf(&macStr[i*2],"%X", mac[i]);
		}
	}
	return true;
}



bool WifiHelper::syncRTCwithSNTP(const char * host){
	char targetHost[] = DEFAULT_SNTP_HOST;
	int32_t retval;
	time_t raw;
	struct tm * timeinfo;
	struct tm timeBuf;
	datetime_t date;
	char datetime_buf[256];

	if (host == NULL){
		retval = WifiHelper::sntp_get_time(targetHost, &raw);
	} else {
		retval = WifiHelper::sntp_get_time(host, &raw);
	}

	if (retval == 0){

		timeinfo = localtime_r( &raw, &timeBuf );
	/*
		printf("TimeInfo(%lld): %d-%d-%d %d:%d:%d\n\r",
				raw,
				timeinfo->tm_year + 1900,
				timeinfo->tm_mon + 1,
				timeinfo->tm_mday,
				timeinfo->tm_hour,
				timeinfo->tm_min,
				timeinfo->tm_sec
				);
	*/

		memset(&date, 0, sizeof(date));
		date.sec = timeinfo->tm_sec;
		date.min = timeinfo->tm_min;
		date.hour = timeinfo->tm_hour;
		date.day = timeinfo->tm_mday;
		date.month = timeinfo->tm_mon + 1;
		date.year = timeinfo->tm_year + 1900;

		rtc_init();
		rtc_set_datetime (&date);

		datetime_to_str(datetime_buf, sizeof(datetime_buf), &date);
		LogInfo(("Time: %s\n\r", datetime_buf));

		return true;
	}

	LogError(("SNTP Error %d", retval));

	return false;
}


bool WifiHelper::isJoined(){
	int32_t ret = Driver_WiFi1.IsConnected();
	return (ret != 0);
}



int32_t WifiHelper::sntp_get_time (const char *server, time_t *seconds) {
  int32_t  socket;
  uint8_t  buf[48];
  uint8_t  ip[4];
  uint32_t ip_len;
  uint32_t timeout;
  int32_t  status;

  printf("SNTP to %s\n", server);
  /* Resolve SNTP/NTP server IP address */
  ip_len = 4U;
  status = iotSocketGetHostByName(server, IOT_SOCKET_AF_INET, ip, &ip_len);
  if (status != 0) {
    return (-1);
  }

  /* Compose SNTP request: vers.3, mode=Client */
  memset(buf, 0, sizeof(buf));
  buf[0] = 0x1B;

  /* Create UDP socket */
  socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);
  if (socket < 0) {
    return (-1);
  }

  /* Set socket receive timeout: 10 seconds */
  timeout = 10000U;
  status = iotSocketSetOpt(socket, IOT_SOCKET_SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (status < 0) {
    iotSocketClose(socket);
    return (-1);
  }

  /* Send SNTP request (port 123) */
  status = iotSocketSendTo(socket, buf, sizeof(buf), ip, sizeof(ip), 123U);
  if (status < 0) {
    iotSocketClose(socket);
    return (-1);
  }

  /* Read SNTP response */
  status = iotSocketRecv(socket, buf, sizeof(buf));
  if (status < 0) {
    iotSocketClose(socket);
    return (-1);
  }

  /* Extract time */
  if (seconds != NULL) {
    *seconds = ((buf[40] << 24) | (buf[41] << 16) | (buf[42] << 8) | buf[43]) - 2208988800U;
  }

  iotSocketClose(socket);

  return 0;
}
