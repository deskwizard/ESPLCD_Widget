#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "defines.h"

extern struct sensorData localSensor;

struct sensorData {
  float temperature;
  float pressure;
  uint8_t humidity;
  uint8_t fetchSuccess = FETCH_FAIL; // 0 = no errors
};

void setupLocalSensor();
void handleLocalSensor();

#endif