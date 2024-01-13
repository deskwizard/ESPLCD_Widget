#ifndef _DATASOURCES_H_
#define _DATASOURCES_H_

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define BUF_SIZE 300     // Buffer size

////////////////////////////// Weather //////////////////////////////
#define URL_BASE_WEATHER "https://api.open-meteo.com/v1/forecast?timeformat=unixtime&"
#define LAT 55.70
#define LONG 61.47

struct weatherData {
  float temp;
  float feels;
  float windSpeed;
  uint16_t windDir;
  uint8_t humidity;
};
extern struct weatherData currentWeather;

////////////////////////////// Moon //////////////////////////////
#define URL_BASE_MOON "https://api.farmsense.net/v1/moonphases/?d="

struct moonData {
  uint8_t fetchSuccess; // 0 = no errors
  uint8_t index = 8;
  float age;
  char phase[40];
  char name[40];
};
extern struct moonData moon;

void handleDataSources();
void updateDataSources();
void updateWeatherData();
void updateMoonData();

#endif