#include "datasources.h"
#include "defines.h"
#include "display.h"
#include "network.h"



void i2cScan();

void setup()
{

  pinMode(LED_DEBUG, OUTPUT);

  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nHellord");

  /*   while (true)
    {
      i2cScan();
    }
  */

  setupTime();
  setupDisplay();
  setupDataSources();

#ifndef NO_NET
  setupWiFi();
#endif
}

void loop()
{

#ifndef NO_NET
  handleWiFi();
  handleNTP();
  handleDataSources();
#endif

  handleDisplay();

  // blink();
  handleSerial();
}

void blink()
{

  uint32_t currentMillis = millis();
  static uint32_t previousMillis = 0;
  static bool ledState;

  if ((uint32_t)(currentMillis - previousMillis) >= 1000)
  {
    ledState = !ledState;
    digitalWrite(LED_DEBUG, ledState);
    previousMillis = currentMillis;
  }
}

void processSyncMessage()
{

  uint32_t pctime = Serial.parseInt();

  if (pctime >= DEFAULT_TIME)
  { // check the integer is a valid time (greater
    // than Jan 1 2013)
    Serial.println(" *** Serial time and RTC update ***");

    // Sync Arduino clock to the time received on the serial port
    setTime(pctime + 18000);
    RTC.set(pctime + 18000);
    serialClockDisplay();
    ESP.restart();
  }
}

void handleSerial()
{

  if (Serial.available())
  {

    unsigned char c = Serial.read();
    int readValue;

    switch (c)
    {

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

void i2cScan()
{

  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
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