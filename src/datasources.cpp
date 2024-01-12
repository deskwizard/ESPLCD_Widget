#include "datasources.h"
#include "defines.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "display.h"

char fetchURL[55];
uint32_t unixTime = 1904249932;

WiFiClientSecure client;
HTTPClient http;

JsonDocument moonInfo; // Allocate the JSON document

uint8_t moonFetchSuccess; // 0 = no errors
uint8_t moonImageIndex = 12;
float moonAge;
char moonPhase[40];
char moonName[40];

void handleDataSources() {


}

void updateDataSources() {

  ///////////////////////// Moon Phase //////////////////////////

  client.setInsecure();

  snprintf(fetchURL, 80, URL_BASE "%u", now());

  // Serial.print("url: ");
  // Serial.println(fetchURL);

  http.begin(client, fetchURL);
  http.GET();

  String fetchedJSON = http.getString();
  fetchedJSON = fetchedJSON.substring(1, fetchedJSON.length() - 1);

  // Serial.println(fetchedJSON);  // Debug

  deserializeJson(moonInfo, fetchedJSON); // Parse response

  http.end(); // Disconnect

  moonFetchSuccess = moonInfo["Error"]; // 0 = no errors

  if (!moonFetchSuccess) {
    moonImageIndex = moonInfo["Index"];
    moonAge = moonInfo["Age"];

    strcpy(moonPhase, moonInfo["Phase"]);
    strcpy(moonName, moonInfo["Moon"][0]);

    // Print the values
    Serial.println(moonPhase);
    Serial.println(moonName);
    Serial.println(moonImageIndex);
    Serial.println(moonAge, 8);
    updateMoonDisplay();
  } else {
    Serial.print("Moon data fetch error: ");
    Serial.println(moonFetchSuccess);
  }
}
