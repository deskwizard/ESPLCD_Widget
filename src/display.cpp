#include "display.h"
#include "defines.h"

void handleDisplay() {

  static uint8_t previousSecond = 0;
  if (timeStatus() != timeNotSet) {
    if (second() != previousSecond) {

      if (second() == 0) {
        Serial.println("minute++");

        if (minute() == 0) {
          Serial.println("hour++");
        }
      }

      previousSecond = second();
      serialClockDisplay();
    }
  }
}

void updateDisplay() {}