/*
 * WifiAgent.h
 *
 *  Created on: 24 Nov 2021
 *      Author: jondurrant
 */

#ifndef SRC_WIFIHELPER_H_
#define SRC_WIFIHELPER_H_

#include "MQTTConfig.h"
#include <stdlib.h>
#include "lwesp/lwesp.h"

#ifndef ESP01_RST_PIN
#define ESP01_RST_PIN 11
#endif

#ifndef ESP01_RST_DELAY
#define ESP01_RST_DELAY 2000
#endif

#ifndef ESP01_SNTP_TIMEZONE
#define ESP01_SNTP_TIMEZONE 0
#endif

#ifndef ESP01_SNTP_LOCAL_HOST
#define ESP01_SNTP_LOCAL_HOST "178.79.160.57"
#endif

class WifiHelper {
public:
	WifiHelper();

	virtual ~WifiHelper();

	/***
	 * Connect to the AP
	 * @param sid - SID
	 * @param passwd - PASSWD
	 * @return - true on success
	 */
	static bool connectToAp(const char * sid, const char *passwd);


	static bool autoJoinOrConfig();

	/***
	 * Get IP address of unit
	 * @param ip - output uint8_t[4]
	 * @return - true if IP addres assigned
	 */
	static bool getIPAddress(uint8_t *ip);

	/***
	 * Get IP address of unit
	 * @param ips - output char * up to 16 chars
	 * @return - true if IP addres assigned
	 */
	static bool getIPAddressStr(char *ips);


	static bool getMACAddressStr(char *macStr);


	static bool syncRTCwithSNTP();

	static bool isJoined();


private:

	static void setupGPIO();

	static void resetESP01();

	static void sntpSetupCB(lwespr_t resp, void* args);

	static void syncRTCCB(lwespr_t resp, void* args);

	static lwesp_datetime_t dateTime;


};

#endif /* SRC_WIFIHELPER_H_ */
