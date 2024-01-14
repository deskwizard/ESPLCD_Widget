#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "credentials.h"
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

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
#define NTP_SHORT_DELAY (MINUTES_TO_MS * 5)

void setupWiFi();
void connectWiFi();
void handleWiFi();
void findLocalNTP();
void getNtpTime();

void sendNTPpacket(IPAddress &address);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiEvent(WiFiEvent_t event); // Debug and not working probably

#endif