
#include "defines.h"
#include "network.h"
#include <Wire.h>

// On Linux, you can use "date +T%s\n > /dev/ttyUSB0" (for example) for UTC time
// zone fake in the past date: T1407788400\n (PM) @ 4:20 fake in the past date:
// T1407745200\n (AM) @ 4:20 before midnight: T1628683170\n before midnight:
// T1628639970\n before noon:     T1628679570\n

#define LED_DEBUG 5

void blink();
void handleSerial();

void setup() {

  pinMode(LED_DEBUG, OUTPUT);

  Wire.begin(32, 33); // SCL - SDA
  Serial.begin(115200);
  Serial.println("\nHellord");

  setupTime();
  setupWiFi();
}

void loop() {

  handleWiFi();
  handleNTP();

  blink();
  handleSerial();
}

void blink() {

  uint32_t currentMillis = millis();
  static uint32_t previousMillis = 0;
  static bool ledState;

  if ((uint32_t)(currentMillis - previousMillis) >= 1000) {
    ledState = !ledState;
    digitalWrite(LED_DEBUG, ledState);
    previousMillis = currentMillis;
  }
}

void processSyncMessage() {

  uint32_t pctime = Serial.parseInt();

  if (pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater
                                // than Jan 1 2013)
    Serial.println(" *** Serial time and RTC update ***");
    setTime(
        pctime +
        18000); // Sync Arduino clock to the time received on the serial port
    RTC.set(pctime + 18000);
    serialClockDisplay();
  }
}

void handleSerial() {

  if (Serial.available()) {

    unsigned char c = Serial.read();
    int readValue;

    switch (c) {

    case 'T': // Set RTC time using serial (UTC)
      processSyncMessage();
      break;

    case '/':
      RTC.set(0);
      ESP.restart();
      break;
    }
  }
}

void i2cScan() {

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(5000); // wait 5 seconds for next scan
}