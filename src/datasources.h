#ifndef _DATASOURCES_H_
#define _DATASOURCES_H_

#include "defines.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#define BUF_SIZE 400 // Buffer size
#define DATA_UPDATE_INTERVAL (MINUTES_TO_MS * 15)
#define DATA_RETRY_INTERVAL (MINUTES_TO_MS * 5)


////////////////////////////// Weather //////////////////////////////
#define URL_BASE_WEATHER                                                       \
  "https://api.open-meteo.com/v1/forecast?timeformat=unixtime&"


struct weatherData {
  uint8_t weatherCode;
  float temperature;
  float feels;
  float precipitation;
  uint8_t cloudCover;
  float windSpeed;
  uint16_t windDir;
  uint8_t humidity;
  bool isDay;
  uint8_t fetchSuccess = FETCH_FAIL; // 0 = no errors
};

extern struct weatherData currentWeather;

////////////////////////////// Moon //////////////////////////////
#define URL_BASE_MOON "https://api.farmsense.net/v1/moonphases/?d="

struct moonData {
  char name[40];
  char phase[40];
  float age;
  float illumination;
  uint8_t index;
  uint8_t fetchSuccess = FETCH_FAIL; // 0 = no errors
};
extern struct moonData moon;

void setupDataSources();
void handleDataSources();

void updateWeatherData();
void updateMoonData();

#endif