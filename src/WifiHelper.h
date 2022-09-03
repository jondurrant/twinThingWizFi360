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

#ifndef DEFAULT_SNTP_HOST
#define DEFAULT_SNTP_HOST "0.uk.pool.ntp.org"
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


	static bool syncRTCwithSNTP(const char * host = NULL);

	static bool isJoined();


private:

	static int32_t sntp_get_time (const char *server, uint32_t *seconds);

};

#endif /* SRC_WIFIHELPER_H_ */
