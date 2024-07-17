#include "network.h"
#include "ntp.h"

WiFiUDP Udp;

extern bool forceNTPFail; // Debug

bool localTimeServerAvailable = false;
bool NTPState = false;

uint8_t packetBuffer[NTP_PACKET_SIZE]; // Buffer to hold in/out packets
uint32_t NtpRetryMillis = 0;
uint32_t NtpUpdateDelay = NTP_DEFAULT_DELAY;

/////////////////// NTP Servers //////////////////
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

    if (WiFi.status() == WL_CONNECTED) {
      getNtpTime();
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
    NtpUpdateDelay = NTP_SHORT_DELAY;
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

  Udp.begin(NTP_LOCAL_PORT); // Start UDP for NTP

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
      NtpUpdateDelay = NTP_DEFAULT_DELAY;

      Serial.println();
      return;
    }
  }

  Serial.print("--- NTP  --- ");
  Serial.println("No NTP Response");
  Serial.println();
  NTPState = false;
  NtpUpdateDelay = NTP_SHORT_DELAY;
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



void findLocalNTP() {

#ifdef NTP_LOCAL_DISC
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
  /*
    Serial.print("Local available: ");
    Serial.println(localTimeServerAvailable);
    Serial.println();
  */
#endif
}
