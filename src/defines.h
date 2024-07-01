#ifndef _DEFINES_H_
#define _DEFINES_H_

//#define NO_NET
//define NO_ANIM
//#define TEST_DISPLAY

#include <Arduino.h>
#include <Wire.h>

#define LED_DEBUG 4

#define PIN_SCL 26
#define PIN_SDA 25

void blink();
void handleSerial();

#endif