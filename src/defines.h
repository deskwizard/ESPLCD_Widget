#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <Arduino.h>
#include <Wire.h>

//#define NO_NET
//#define NO_ANIM
//#define TEST_DISPLAY

#define LED_DEBUG 4

#define PIN_SCL 26
#define PIN_SDA 25

#define FETCH_OK 0
#define FETCH_FAIL 42

void blink();
void handleSerial();

#endif