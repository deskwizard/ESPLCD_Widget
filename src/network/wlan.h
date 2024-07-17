#ifndef _WLAN_H_
#define _WLAN_H_

#include "defines.h"
#include "network.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

void setupWiFi();
void connectWiFi();
void handleWiFi();

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiEvent(WiFiEvent_t event); // Debug and not working probably?

#endif