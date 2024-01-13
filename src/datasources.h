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
  float temp = -42.69;
  float feels;
  float windSpeed;
  uint16_t windDir;
  uint8_t humidity;
  uint8_t fetchSuccess = 42; // 0 = no errors
};
extern struct weatherData currentWeather;

////////////////////////////// Moon //////////////////////////////
#define URL_BASE_MOON "https://api.farmsense.net/v1/moonphases/?d="

struct moonData {
  uint8_t index;
  float age;
  char phase[40];
  char name[40];
  uint8_t fetchSuccess = 42; // 0 = no errors
};
extern struct moonData moon;

void handleDataSources();
void updateDataSources();
void updateWeatherData();
void updateMoonData();

#endif