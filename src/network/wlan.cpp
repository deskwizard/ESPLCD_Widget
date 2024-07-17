#include "network.h"
#include "defines.h"

extern const char ssid[];
extern const char password[];
extern const char deviceHostname[];

uint8_t connectionRetryCounter = 0;
uint32_t WiFiRetryMillis = 0;
uint32_t WiFiRetryDelay = MINUTES_TO_MS * 5;

void setupWiFi() {

  // Deletes previous configuration
  WiFi.disconnect(true);

  delay(1000);

  // Register WiFi any event callback function
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiDisconnected,
               WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  connectWiFi();
}

void connectWiFi() {
  WiFi.disconnect(true);
  setCpuFrequencyMhz(160);
  WiFi.mode(WIFI_AP);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(ssid, password);
  WiFi.setHostname(deviceHostname);
}

void disconnectWiFi() {
  WiFi.mode(WIFI_OFF);
  Serial.println();
  Serial.println("WiFi disconnect called");
  MDNS.end();
}

void handleWiFi() {

  uint32_t currentMillis = millis();
  if ((uint32_t)(currentMillis - WiFiRetryMillis) >= WiFiRetryDelay &&
      WiFiRetryMillis != 0) {
    Serial.print(">>> WiFi --- ");
    Serial.println("Timeout - Attempting connection");
    connectWiFi();
    WiFiRetryMillis = currentMillis;
  }
}

// ***** WiFi Events callbacks *****

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {

  Serial.print(">>> WiFi --- ");
  Serial.print("Received IP Address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));

  if (!MDNS.begin("ESP32_Browser")) {
    Serial.println("Error setting up MDNS responder!");
  }

  //delay(500);
  //findLocalNTP();

  //getNtpTime();

} // got ip

void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info) {

  Serial.print(">>> WiFi --- ");
  Serial.print("WiFi connected to: ");
  Serial.println(ssid);

  connectionRetryCounter = 0;
  WiFiRetryMillis = 0;
} // connected

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {

  Serial.print(">>> WiFi --- ");
  Serial.print("WiFi disconnected");

  connectWiFi();

  connectionRetryCounter++;

  Serial.print(" | Tries: ");
  Serial.println(connectionRetryCounter);

  // If we retried X times, stop trying
  if (connectionRetryCounter == NTP_MAX_RETRY) {
    Serial.print(">>> WiFi --- ");
    Serial.println("Connection attempt failed");
    Serial.println();

    WiFi.mode(WIFI_OFF);

    connectionRetryCounter = 0;
    WiFiRetryMillis = millis();
  }
}



// Debug

void WiFiEvent(WiFiEvent_t event) {

  // debug
  return;

  Serial.printf("[WiFi-event] event: %d: ", event);

  switch (event) {
  case SYSTEM_EVENT_WIFI_READY:
    Serial.println("WiFi interface ready");
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    Serial.println("Completed scan for access points");
    break;
  case SYSTEM_EVENT_STA_START:
    Serial.println("WiFi client started");
    break;
  case SYSTEM_EVENT_STA_STOP:
    Serial.println("WiFi clients stopped");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Connected to access point");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from WiFi access point");
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    Serial.println("Authentication mode of access point has changed");
    break;

  case SYSTEM_EVENT_STA_GOT_IP:
    //      Serial.print("Obtained IP address: ");
    //      Serial.println(WiFi.localIP());
    break;

  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println("Lost IP address and IP address is reset to 0");
    break;

  case SYSTEM_EVENT_GOT_IP6:
    Serial.println("IPv6 is preferred");
    break;

  default:
    break;
  }
}