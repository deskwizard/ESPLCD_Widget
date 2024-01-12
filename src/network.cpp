#include "network.h"
#include "ArduinoJson.h"
#include "credentials.h"
#include "datasources.h"
#include "defines.h"
#include <ESPmDNS.h>
#include <WiFi.h>

extern bool forceNTPFail;

void sendNTPpacket(IPAddress &address);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiEvent(WiFiEvent_t event); // Debug and not working probably

// Choose one of these
#define NTP_LOCAL_DISC
// #define NTP_LOCAL
// #define REM_STATIC
// #define REM_HOSTNAME
// #define REM_POOL

bool localTimeServerAvailable = false;

#include <WiFiUdp.h>
WiFiUDP Udp;

extern const char ssid[];
extern const char password[];
extern const char hostname[];

uint8_t connectionRetryCounter = 0;
const uint8_t maxRetryCount =
    10; // Number of tries it will make to connect before giving up, tries are
        // around 2sec appart
uint32_t WiFiRetryMillis = 0;
uint32_t WiFiRetryDelay = MINUTES_TO_MS * 5;

bool NTPState = false;
const int16_t NTP_PACKET_SIZE =
    48; // NTP time is in the first 48 bytes of message
uint8_t
    packetBuffer[NTP_PACKET_SIZE]; // Buffer to hold incoming & outgoing packets
uint16_t localPort = 8888;         // Local port to listen for UDP packets
uint32_t NtpRetryMillis = 0;
const uint32_t NtpUpdateDefaultDelay = MINUTES_TO_MS * 5;
const uint32_t NtpUpdateShortDelay = MINUTES_TO_MS * 5;
uint32_t NtpUpdateDelay = NtpUpdateDefaultDelay;

// NTP Servers:

#ifdef NTP_LOCAL_DISC
char ntpServerName[64] = "";
IPAddress ntpServerIP(0, 0, 0, 0);
static const char ntpServerPoolName[] = "us.pool.ntp.org";
#endif

#ifdef NTP_LOCAL
static const char ntpServerName[] = "local server";
IPAddress ntpServerIP(10, 0, 0, 20);
#endif

#ifdef REM_STATIC
static const char ntpServerName[] = "ntp1.torix.ca";
IPAddress ntpServerIP(206, 108, 0, 131);
#endif

#ifdef REM_HOSTNAME
static const char ntpServerName[] = "ntp1.torix.ca";
#endif

#ifdef REM_POOL
static const char ntpServerName[] = "us.pool.ntp.org";
#endif

// static const char ntpServerName[] = "us.pool.ntp.org";
// static const char ntpServerName[] = "time.nist.gov";
// static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
// static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
// static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

void setupWiFi() {

  // Delete old config
  WiFi.disconnect(true);

  delay(1000);

  // Register WiFi any event callback function
  WiFi.onEvent(WiFiEvent);

  WiFi.onEvent(WiFiGotIP,
               WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP); // Register GOT_IP
                                                            // event function
  WiFi.onEvent(
      WiFiConnected,
      WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED); // Register CONNECTED
                                                      // event function
  WiFi.onEvent(
      WiFiDisconnected,
      WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED); // Register
                                                         // DISCONNECTED event
                                                         // function

  connectWiFi();
}

void connectWiFi() {
  WiFi.disconnect(true);
  setCpuFrequencyMhz(160);
  WiFi.mode(WIFI_AP);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(ssid, password);
  WiFi.setHostname(hostname);
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
    while (1) {
      delay(1000);
    }
  }
  delay(500);
  findLocalNTP();

  getNtpTime();

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
  if (connectionRetryCounter == maxRetryCount) {
    Serial.print(">>> WiFi --- ");
    Serial.println("Connection attempt failed");
    Serial.println();

    WiFi.mode(WIFI_OFF);

    connectionRetryCounter = 0;
    WiFiRetryMillis = millis();
  }
}

// ********************** NTP ********************************

void manualNTPUpdate() { NtpRetryMillis = millis() - NtpUpdateDelay; }

void handleNTP() {

  uint32_t currentMillis = millis();

  if ((uint32_t)(currentMillis - NtpRetryMillis) >= NtpUpdateDelay) {
    Serial.println();
    Serial.print("--- NTP  --- ");
    Serial.println("NTP Update Timeout at ");
    serialClockDisplay();
    Serial.println();

    if (WiFi.status() ==
        WL_CONNECTED) { //////////////////////////////////////////////////////////////////////////////
      getNtpTime();
      updateDataSources();
    } else {
      Serial.println("---- WiFi disconnected, attempting connection...");
      connectWiFi();
    }

    NtpRetryMillis = currentMillis;
  }
}

void getNtpTime() {

  NtpRetryMillis = millis();

  if (forceNTPFail == true) {
    Serial.print("--- NTP  --- ");
    Serial.println("Volontary failure");
    NTPState = false;
    NtpUpdateDelay = NtpUpdateShortDelay;
    return;
  }

  while (Udp.parsePacket() > 0)
    ; // Discard any previously received packets

  Serial.println("--- NTP  --- ");
  //  Serial.print("Transmitting NTP Request to: ");

#if defined(REM_HOSTNAME) || defined(REM_POOL)
  // Get a random server from the pool
  IPAddress ntpServerIP; // NTP server's ip address
  WiFi.hostByName(ntpServerName, ntpServerIP);
#endif

#ifdef NTP_LOCAL_DISC

  // Serial.println();

  if (!localTimeServerAvailable) {
    Serial.println("Local server unavailable, using remote pool server...");

    // ntpServerName = "us.pool.ntp.org";
    // WiFi.hostByName(ntpServerName, ntpServerIP);
    //     WiFi.hostByName("us.pool.ntp.org", ntpServerIP);

    // ntpServerName = ntpServerPoolName;
    strcpy(ntpServerName, ntpServerPoolName);
    WiFi.hostByName(ntpServerName, ntpServerIP);

  } else {
    Serial.println("Local server available, using local server...");
  }

#endif

  Serial.print("Transmitting NTP Request to: ");
  Serial.print(ntpServerName);
  Serial.print(" (");
  Serial.print(ntpServerIP);
  Serial.println(")...");

  Udp.begin(localPort); // Start UDP for NTP

  sendNTPpacket(ntpServerIP);

  uint32_t beginWait = millis();

  while (millis() - beginWait < 1500) {

    int size = Udp.parsePacket();

    if (size >= NTP_PACKET_SIZE) {

      Serial.println();
      Serial.println("--- NTP  --- ");
      Serial.println("Received NTP Response, Setting Time.");

      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
      uint32_t secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (uint32_t)packetBuffer[40] << 24;
      secsSince1900 |= (uint32_t)packetBuffer[41] << 16;
      secsSince1900 |= (uint32_t)packetBuffer[42] << 8;
      secsSince1900 |= (uint32_t)packetBuffer[43];

      time_t ntpTime = secsSince1900 - 2208988800UL;

      Serial.print("RTC updating.... ");
      if (RTC.set(ntpTime) == 0) {
        Serial.println("Failed");
      } else {
        Serial.println("Success");
      }

      NTPState = true;
      setTime(ntpTime);
      NtpUpdateDelay = NtpUpdateDefaultDelay;

      Serial.println();
      return;
    }
  }

  Serial.print("--- NTP  --- ");
  Serial.println("No NTP Response");
  Serial.println();
  NTPState = false;
  NtpUpdateDelay = NtpUpdateShortDelay;
  // disconnectWiFi();
}

// Sends an NTP request to the time server at the given IP address
void sendNTPpacket(IPAddress &address) {

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
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

void findLocalNTP() {

  Serial.println();
  Serial.println("Searching for local NTP server...");

  int n = MDNS.queryService("ntp", "udp");

  if (n == 0) {
    Serial.println("No local NTP server found");
    Serial.println();
    localTimeServerAvailable = false;
  } else {
    Serial.print(n);
    Serial.println(" local NTP server(s) found");

    for (int i = 0; i < n; ++i) {
      // Print details for each service found
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(": ");

      MDNS.hostname(i).toCharArray(ntpServerName, sizeof(MDNS.hostname(i)));
      Serial.print(ntpServerName);
      //      Serial.print(MDNS.hostname(i));
      Serial.print(" (");

      //      NTP_SERVER_LOCAL = MDNS.IP(i);
      //      Serial.print(NTP_SERVER_LOCAL);

      ntpServerIP = MDNS.IP(i);
      Serial.print(ntpServerIP);

      //      Serial.print(MDNS.IP(i));

      Serial.print(":");

      //      ntpServerName = MDNS.hostname(i);
      //      Serial.println(ntpServerName);

      //      MDNS.hostname(i).toCharArray(ntpServerName, sizeof
      //      (MDNS.hostname(i)));

      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
    Serial.println();
    //
    localTimeServerAvailable = true;
  }

  Serial.print("Local available: ");
  Serial.println(localTimeServerAvailable);
  Serial.println();
}