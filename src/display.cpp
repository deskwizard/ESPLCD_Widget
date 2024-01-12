#include "display.h"
#include "defines.h"
#include <TimeLib.h>
#include <Timezone.h>

#include <SPI.h>
#include <TFT_eSPI.h>

#include "fonts/NotoSans12p.h"
#include "fonts/NotoSans42p.h"
#include "fonts/NotoSans70p.h"
#include "fonts/moonFont20p.h"
#include "fonts/qweather20p.h"
#define FONT_SMALL &NotoSans_Regular12pt7b
#define FONT_TIME &NotoSans_Regular70pt7b
#define FONT_COLON &NotoSans_Regular42pt7b
#define FONT_MOON_ICON &MoonPhases20pt7b
#define FONT_WEATHER_ICON &qweather_icons20pt7b

TFT_eSPI tft = TFT_eSPI();

extern time_t localTime;
extern Timezone myTZ;

/**********************************************************************************/
#include "never.h" // Image is stored here in an 8 bit array
#include <PNGdec.h>

PNG png; // PNG decoder inatance

#define IMG_MAX_W 240 // Adjust for your images
#define IMG_X 40
#define IMG_Y 0
void pngDraw(PNGDRAW *pDraw);
/**********************************************************************************/

// uint8_t moonIndex = 8;
extern uint8_t moonImageIndex;

const uint8_t moonMap[29] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                             'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                             'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B'};

char dateString[50];

void drawStatic() {

  // The ':' in the middle
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(FONT_COLON);
  tft.drawChar(':', COLON_X_OFFSET, COLON_Y_OFFSET, GFXFF);

  tft.drawFastHLine(0, TOP_LINE_Y - 10, DISP_W, TFT_ORANGE);
  tft.drawFastHLine(0, TOP_LINE_Y - 11, DISP_W, TFT_ORANGE);
  tft.drawFastHLine(0, TOP_LINE_Y - 12, DISP_W, TFT_ORANGE);

  tft.drawFastHLine(0, BTM_LINE_Y, DISP_W, TFT_ORANGE);
  tft.drawFastHLine(0, BTM_LINE_Y + 1, DISP_W, TFT_ORANGE);
  tft.drawFastHLine(0, BTM_LINE_Y + 2, DISP_W, TFT_ORANGE);
}

void setupDisplay() {

  pinMode(TFT_BACKLIGHT, OUTPUT);

  tft.begin();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);

  digitalWrite(TFT_BACKLIGHT, LOW);

  drawStatic();
  updateMoonDisplay();
  updateTimeDisplay();
  updateDateString();
  updateDateDisplay();

  Serial.println("-------------------------------------------");
  Serial.println();
}

void handleDisplay() {

  static uint8_t previousMinute = 0;

  if (timeStatus() != timeNotSet) {

    localTime = myTZ.toLocal(now());

    if (minute(localTime) != previousMinute) {

      // Serial.print("---- M: ");
      // Serial.println(minute(localTime));

      updateMinutesDisplay();
      previousMinute = minute(localTime);

      if (minute(localTime) == 0) {

        updateHoursDisplay();

        if (hour(localTime) == 0) {
          updateDateString();
          updateDateDisplay();
          updateMoonDisplay();
        }
      }
    }
  }
}

void updateMoonDisplay() {
  // Serial.println("updateMoonDisplay() called.");
  //  Serial.println(moonIndex);

  if (moonImageIndex != 29) {
    tft.setViewport(VP_MOON_ICON_X, VP_MOON_ICON_Y, VP_MOON_ICON_W,
                    VP_MOON_ICON_H);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_MOON_ICON);
    tft.drawChar(moonMap[moonImageIndex], 0, 32, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_WHITE);
    tft.drawCircle(19, 18, 16, TFT_WHITE);
    tft.resetViewport();
  } //

  else {
    moonImageIndex = 0;
    /*
        Serial.println("------");
        tft.fillScreen(TFT_BLACK);
        neverGive();
        delay(5000);

        tft.fillScreen(TFT_BLACK);
        delay(5000);

        drawStatic();

        updateTimeDisplay();
        updateDateDisplay();

        updateMoonDisplay();
        */
  }
}

void updateTimeDisplay() {
  updateHoursDisplay();
  updateMinutesDisplay();
}

void updateHoursDisplay() {
  // Serial.println("updateHoursDisplay() called.");

  localTime = myTZ.toLocal(now());

  uint8_t tens = hour(localTime);
  uint8_t ones = tens % 10;
  tens = (tens / 10) % 10;

  tft.setFreeFont(FONT_TIME);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  // TViewports
  tft.setViewport(VP_TIME_X, VP_TIME_Y, VP_TIME_W, VP_TIME_H);
  if (tens != 0) {
    tft.drawNumber(tens, 0, 0, GFXFF);
  } //
  else {
    tft.drawChar(' ', 0, 0, GFXFF);
  }
  tft.resetViewport();

  tft.setViewport(VP_TIME_W, VP_TIME_Y, VP_TIME_W, VP_TIME_H);
  tft.drawNumber(ones, 0, 0, GFXFF);
  tft.resetViewport();
}

void updateMinutesDisplay() {
  // Serial.println("updateMinutesDisplay() called.");

  localTime = myTZ.toLocal(now());

  uint8_t tens = minute(localTime);
  uint8_t ones = tens % 10;
  tens = tens / 10;
  tens = tens % 10;

  tft.setFreeFont(FONT_TIME);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  tft.setViewport(DISP_W - (VP_TIME_W * 2), VP_TIME_Y, VP_TIME_W, VP_TIME_H);
  tft.drawNumber(tens, 0, 0, GFXFF);
  tft.resetViewport();

  tft.setViewport(DISP_W - VP_TIME_W, VP_TIME_Y, VP_TIME_W, VP_TIME_H);
  tft.drawNumber(ones, 0, 0, GFXFF);
  tft.resetViewport();
}

void updateDateString() {

  Serial.println("updateDateString() called.");

  char tempBuffer[5];
  localTime = myTZ.toLocal(now());

  strcpy(dateString, dayShortStr(weekday(localTime)));
  strcat(dateString, ". ");
  strcat(dateString, monthStr(month(localTime)));
  strcat(dateString, " ");
  strcat(dateString, itoa(day(localTime), tempBuffer, 10));

  switch (day(localTime)) {
  case 1:
    strcat(dateString, "st ");
    break;
  case 2:
    strcat(dateString, "nd ");
    break;
  case 3:
    strcat(dateString, "rd ");
    break;
  default:
    strcat(dateString, "th ");
  }

  strcat(dateString, itoa(year(localTime), tempBuffer, 10));

  Serial.println(dateString);
}

void updateDateDisplay() {
  Serial.println("updateDateDisplay() called.");
  tft.setFreeFont(FONT_SMALL);
  tft.setViewport(0, VP_DATE_Y, VP_DATE_W, VP_DATE_H);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString(dateString, 160, 0, GFXFF);
  tft.resetViewport();
}

///////////////////////////////////////////////////////////////////////////////////////////
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[IMG_MAX_W];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(IMG_X, IMG_Y + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void neverGive() {
  int16_t rc = png.openFLASH((uint8_t *)never, sizeof(never), pngDraw);
  if (rc == PNG_SUCCESS) {
    Serial.println("Successfully opened png file");
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n",
                  png.getWidth(), png.getHeight(), png.getBpp(),
                  png.getPixelType());
    tft.startWrite();
    uint32_t dt = millis();
    rc = png.decode(NULL, 0);
    Serial.print(millis() - dt);
    Serial.println("ms");
    tft.endWrite();
    // png.close(); // not needed for memory->memory decode
  }
}