// https://www.codeproject.com/Articles/5336116/Img2Cpp-Create-Cplusplus-Headers-for-Embedding-Ima
// https://notisrac.github.io/FileToCArray/

/******** WARNING: INCLUDE ORDER MATTERS !!! ********/
#include "display.h"
#include "datasources.h"
#include "defines.h"

#define MAX_IMAGE_WIDTH 240 // Adjust for your images
File pngfile;
PNG png;
int16_t xpos = 0;
int16_t ypos = 0;

#define FONT_SMALL &NotoSans_Regular12pt7b
#define FONT_MED1 &NotoSans_Regular14pt7b
#define FONT_MED2 &NotoSans_Regular20pt7b
#define FONT_TIME &NotoSans_Regular70pt7b
#define FONT_COLON &NotoSans_Regular42pt7b

TFT_eSPI tft = TFT_eSPI();

extern time_t localTime;
extern Timezone myTZ;

char dateString[50];

void setupDisplay() {

  // Initialise FS
  if (!FileSys.begin()) {
    Serial.println("LittleFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }

  pinMode(TFT_BACKLIGHT, OUTPUT);
  ledcAttachPin(TFT_BACKLIGHT, PWM1_CH);
  ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);

  // TODO: Preload brightness and adjust ASAP
  for (int8_t x; x <= CDS_AVG_COUNT; x++) {
    Serial.println(x);
    handleBacklight();
  }

  tft.begin();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  // tft.fillScreen(TFT_PURPLE);

  drawStatic();

  updateTimeDisplay();
  updateDateString();
  updateDateDisplay();
  updateMoonDisplay();
  updateWeatherDisplay();

  // Serial.println("-------------------------------------------");
  // Serial.println();
}

void drawStatic() {

  // The ':' in the middle
  tft.setTextColor(TIME_COLOR, TFT_BLACK);
  tft.setFreeFont(FONT_COLON);
  tft.drawChar(':', COLON_X_OFFSET, COLON_Y_OFFSET, GFXFF);

  tft.drawFastHLine(0, TOP_LINE_Y, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, TOP_LINE_Y - 1, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, TOP_LINE_Y - 2, DISP_W, H_LINE_COLOR);

  tft.drawFastHLine(0, BTM_LINE_Y, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, BTM_LINE_Y + 1, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, BTM_LINE_Y + 2, DISP_W, H_LINE_COLOR);
}

void updateWeatherDisplay() {

  char buffer[50];

  tft.setViewport(VP_WEA_X, VP_WEA_Y, VP_WEA_W, VP_WEA_H);

  // tft.fillScreen(TFT_BLUE);
  tft.fillScreen(TFT_BLACK);

  // tft.setTextColor(WEATHER_COLOR, TFT_NAVY);
  tft.setTextColor(WEATHER_COLOR, TFT_BLACK);

  if (currentWeather.fetchSuccess == 0) {
    tft.setFreeFont(FONT_SMALL);
    snprintf(buffer, 50, "%.0f°C / %.0f°C    %d", currentWeather.temp,
             currentWeather.feels, currentWeather.weatherCode);
  } //
  else {
    tft.setFreeFont(FONT_SMALL);
    snprintf(buffer, 50, "Fetching...");
  }

  tft.drawString(buffer, 0, 0, GFXFF);

  tft.resetViewport();
}

void updateWeatherIcon() {

  char imageName[80];

  snprintf(imageName, 80, "/weather/small/%d/%d.png", currentWeather.isDay,
           currentWeather.weatherCode);

  Serial.print("image filename: ");
  Serial.println(imageName);

  xpos = WEA_ICON_X;
  ypos = WEA_ICON_Y;

  int16_t rc =
      png.open(imageName, pngOpen, pngClose, pngRead, pngSeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n",
                  png.getWidth(), png.getHeight(), png.getBpp(),
                  png.getPixelType());

    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
    } else {
      rc = png.decode(NULL, 0);
      png.close();
    }
    tft.endWrite();
  }
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
/*
void updateMoonDisplay1() {
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
  } //
  else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_MED2);
    tft.drawChar(moonMap[moon.index], 0, 32, GFXFF);
    tft.drawCircle(19, 18, 15, TFT_WHITE);
    tft.drawCircle(19, 18, 16, TFT_WHITE);
  }
  tft.resetViewport();
}
*/
void updateMoonDisplay() {

  char imageName[80];
  // snprintf(imageName, 80, "/weather/small/night/%d.png", index);
  snprintf(imageName, 80, "/moon/small/%d.png", moon.index);

  Serial.print("image filename: ");
  Serial.println(imageName);

  xpos = VP_MOON_ICON_X;
  ypos = VP_MOON_ICON_Y;

  int16_t rc =
      png.open(imageName, pngOpen, pngClose, pngRead, pngSeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();

    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n",
                  png.getWidth(), png.getHeight(), png.getBpp(),
                  png.getPixelType());

    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
    } else {
      rc = png.decode(NULL, 0);
      png.close();
    }
    tft.endWrite();
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
  tft.setTextColor(TIME_COLOR, TFT_BLACK);

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

  tft.setTextColor(TIME_COLOR, TFT_BLACK);

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
  tft.setTextColor(DATE_COLOR, TFT_BLACK);
  tft.drawCentreString(dateString, 160, 0, GFXFF);
  tft.resetViewport();
}

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.

void *pngOpen(const char *filename, int32_t *size) {
  Serial.printf("Attempting to open %s\n", filename);
  pngfile = FileSys.open(filename, "r");
  *size = pngfile.size();
  return &pngfile;
}

void pngClose(void *handle) {
  File pngfile = *((File *)handle);
  if (pngfile)
    pngfile.close();
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!pngfile)
    return 0;
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
  if (!pngfile)
    return 0;
  page = page; // Avoid warning
  return pngfile.seek(position);
}