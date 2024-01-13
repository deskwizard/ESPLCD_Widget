#include "display.h"
#include "datasources.h"
#include "defines.h"

#define FONT_SMALL &NotoSans_Regular12pt7b
#define FONT_MED1 &NotoSans_Regular14pt7b
#define FONT_MED2 &NotoSans_Regular20pt7b
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

PNG png; // PNG decoder instance

#define IMG_MAX_W 240 // Adjust for your images
#define IMG_X 40
#define IMG_Y 0
void pngDraw(PNGDRAW *pDraw);
/**********************************************************************************/

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

  // Serial.print("index: ");
  // Serial.println(moon.index);

  pinMode(TFT_BACKLIGHT, OUTPUT);
  ledcAttachPin(TFT_BACKLIGHT, PWM1_CH);
  ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);

  tft.begin();
  tft.setRotation(3);

  // tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_PURPLE);

  drawStatic();

  updateTimeDisplay();
  updateDateString();
  updateDateDisplay();
  updateMoonDisplay();
  updateWeatherDisplay();

  // Serial.println("-------------------------------------------");
  // Serial.println();
}

void updateWeatherDisplay() {

  char buffer[50];

  tft.setViewport(VP_WEA_X, VP_WEA_Y, VP_WEA_W, VP_WEA_H);
  tft.fillScreen(TFT_BLUE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (currentWeather.fetchSuccess == 0) {
    tft.setFreeFont(FONT_MED1);
    snprintf(buffer, 50, "%.0f (%.0f)", currentWeather.temp,
             currentWeather.feels);
  } else {
    tft.setFreeFont(FONT_SMALL);
    snprintf(buffer, 50, "No Data Available");
  }

  tft.drawString(buffer, 0, 5, GFXFF);
  /*
    // tft.drawString("-20°C", 9, 29, GFXFF);
    tft.drawFloat(currentWeather.temp, 0, 0, 0, GFXFF);
    // tft.drawString("°C", 9, 29, GFXFF);
    tft.drawString("(", 44, 0, GFXFF);

    tft.drawFloat(currentWeather.feels, 0, 52, 0, GFXFF);
    tft.drawString(")", 94, 0, GFXFF);
  */

  tft.resetViewport();
}

void handleDisplay() {

  static uint8_t previousMinute = 0;

  if (timeStatus() != timeNotSet) {

    localTime = myTZ.toLocal(now());

    if (minute(localTime) != previousMinute) {

      serialClockDisplay();

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
  } // Time set

  handleBacklight();
}

void handleBacklight() {

  uint16_t outputPWMValue;

  static uint16_t readings[CDS_AVG_COUNT] = {
      0};                       // the readings from the analog input
  static uint8_t readIndex = 0; // the index of the current reading
  static uint32_t total = 0;    // the running total
  static uint16_t average = 0;

  uint32_t currentMillis = millis();
  static uint32_t lastReadReadMillis = 0;

  if ((uint32_t)(currentMillis - lastReadReadMillis) >= CDS_READ_RATE) {

    total = total - readings[readIndex];
    readings[readIndex] = analogRead(CDS_PIN) >> 2;
    total = total + readings[readIndex];
    readIndex = readIndex + 1;

    if (readIndex >= CDS_AVG_COUNT) {
      readIndex = 0;
    }

    average = total / CDS_AVG_COUNT;

    if (average > PWM_MINIMUM) {
      outputPWMValue = PWM_MINIMUM;
    } else {
      outputPWMValue = average;
    }

    // Serial.println(average);
    // Serial.println(outputPWMValue);

    // Check for limits before writing
    ledcWrite(PWM1_CH, outputPWMValue);

    lastReadReadMillis = currentMillis;
    // Serial.println();
  }
}

void updateMoonDisplay() {
  // Serial.println("updateMoonDisplay() called.");
  //  Serial.println(moonIndex);

  tft.setViewport(VP_MOON_ICON_X, VP_MOON_ICON_Y, VP_MOON_ICON_W,
                  VP_MOON_ICON_H);
  tft.fillScreen(TFT_BLACK);

  if (moon.fetchSuccess != 0) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setFreeFont(FONT_MED2);
    tft.drawChar('x', 9, 29, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_RED);
    tft.drawCircle(19, 18, 16, TFT_RED);
  } else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_MOON_ICON);
    tft.drawChar(moonMap[moon.index], 0, 32, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_WHITE);
    tft.drawCircle(19, 18, 16, TFT_WHITE);
  }
  /*
  if (moon.index != 42) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_MOON_ICON);
    tft.drawChar(moonMap[moon.index], 0, 32, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_WHITE);
    tft.drawCircle(19, 18, 16, TFT_WHITE);
  }

  else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setFreeFont(FONT_MED);
    tft.drawChar('x', 9, 29, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_RED);
    tft.drawCircle(19, 18, 16, TFT_RED);
    /*
    moonImageIndex = 0;

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

  }
*/
  tft.resetViewport();
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
    tft.fillScreen(TFT_BLACK);
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