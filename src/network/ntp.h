#ifndef _NTP_H_
#define _NTP_H_

#include "network.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

// Choose one of these
// #define NTP_LOCAL_DISC
// #define NTP_LOCAL
// #define REM_STATIC
// #define REM_HOSTNAME
#define REM_POOL

#define NTP_LOCAL_PORT 8888 // Local port to listen for UDP packets
#define NTP_PACKET_SIZE 48
#define NTP_MAX_RETRY 10 // Retries every 2 seconds or thereabout
#define NTP_DEFAULT_DELAY (MINUTES_TO_MS * 30)
//#define NTP_DEFAULT_DELAY (MINUTES_TO_MS * 1)
#define NTP_SHORT_DELAY (MINUTES_TO_MS * 5)

//void findLocalNTP();
void getNtpTime();
void handleNTP();
void sendNTPpacket(IPAddress &address);

#endif