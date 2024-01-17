// https://www.codeproject.com/Articles/5336116/Img2Cpp-Create-Cplusplus-Headers-for-Embedding-Ima
// https://notisrac.github.io/FileToCArray/

/******** WARNING: INCLUDE ORDER MATTERS !!! ********/
#include "display.h"
#include "datasources.h"
#include "defines.h"

#define MAX_IMAGE_WIDTH 300 // Adjust for your images
File pngfile;
PNG png;
int16_t xpos = 0;
int16_t ypos = 0;

bool djph = false;

#define FONT_SMALL &NotoSans_Regular12pt7b
#define FONT_MED1 &NotoSans_Regular14pt7b
#define FONT_MED2 &NotoSans_Regular20pt7b
#define FONT_TIME &NotoSans_Regular70pt7b
#define FONT_COLON &NotoSans_Regular42pt7b

TFT_eSPI tft = TFT_eSPI();

extern time_t localTime;
extern Timezone myTZ;

char dateString[50];

#define beat_delay 400
#define WEATHER_ANIM_FR 200
#define X_OFFSET 140

void animateWeather() {

  char imageName1[80] = "/weather/small/sun.png";
  char imageName2[80] = "/weather/small/moon.png";
  int16_t rc;

  Serial.println("------- Done with the sun, now for the moon! -------");

  // Reset image display location
  xpos = 0;
  ypos = 0;

  Serial.print("image filename: ");
  Serial.print(imageName2);

  // Load moon
  rc = png.open(imageName2, pngOpen, pngClose, pngRead, pngSeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    Serial.printf(" -  Image specs: (%d x %d), %d bpp, pixel type: %d\n",
                  png.getWidth(), png.getHeight(), png.getBpp(),
                  png.getPixelType());

    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
      while (1)
        ;
    }
  }

  // Enter viewport
  tft.setViewport(VP_WEA_ICON_X, VP_WEA_ICON_Y, VP_WEA_ICON_W, VP_WEA_ICON_H, false);
  tft.fillScreen(TFT_PURPLE);

  xpos = VP_WEA_ICON_X - png.getWidth();
  ypos = VP_WEA_ICON_H - png.getHeight();//100;

  // move it around
  for (uint8_t x = xpos; x <= 80 + (X_OFFSET - png.getWidth()); x = (x + 2)) {
    // Serial.println(x);
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
    xpos = x;
    if (xpos <= (X_OFFSET - png.getWidth()) + 40) {
      ypos--;
    } else {
      ypos++;
    }
    delay(WEATHER_ANIM_FR);
  }
  png.close();

  tft.resetViewport();

  // start over
  /*
    djph = true;
    updateMoonDisplay(30);
    updateWeatherDisplay();
   */
}

void setupDisplay() {

  // Initialise FS
  if (!FileSys.begin()) {
    Serial.println("LittleFS initialisation failed!");
    while (1)
      yield();
  }

  pinMode(TFT_BACKLIGHT, OUTPUT);
  ledcAttachPin(TFT_BACKLIGHT, PWM1_CH);
  ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);

  // TODO: Preload brightness and adjust ASAP
  for (int8_t x; x <= CDS_AVG_COUNT; x++) {
    // Serial.println(x);
    handleBacklight();
  }

  tft.begin();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  // tft.fillScreen(TFT_PURPLE);

  drawStatic();
  updateMoonDisplay(29); ////////////////////////////////////////////////////
  updateWeatherDisplay();
  updateTimeDisplay();
  updateDateString();
  updateDateDisplay();

#ifdef TEST_DISPLAY

  while (1) {
    animateWeather();
  }

  uint8_t id = 0;

  char imageName2[80] = "/weather/small/sun.png";
  char imageName1[80] = "/weather/small/moon.png";

  Serial.print("image filename: ");
  Serial.print(imageName1);

  int16_t rc =
      png.open(imageName1, pngOpen, pngClose, pngRead, pngSeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    // tft.startWrite();
    Serial.printf(" -  Image specs: (%d x %d), %d bpp, pixel type: %d\n",
                  png.getWidth(), png.getHeight(), png.getBpp(),
                  png.getPixelType());

    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
      while (1)
        ;
    }
  }

  xpos = 0;
  ypos = 0;

  while (1) {

    char buffer[50];

    tft.setViewport(VP_WEA_ICON_X, VP_WEA_ICON_Y, VP_WEA_ICON_W, VP_WEA_ICON_H);
    tft.fillScreen(TFT_BLUE);

    while (1) {
      /*
            tft.startWrite();
            rc = png.decode(NULL, 0);
            tft.endWrite();
       */
      for (uint8_t x = xpos; x <= 60; x = (x + 2)) {
        // Serial.println(x);
        tft.startWrite();
        rc = png.decode(NULL, 0);
        tft.endWrite();
        xpos = x;
        /*
                tft.startWrite();
                rc = png.decode(NULL, 0);
                tft.endWrite();
                 */
        delay(beat_delay);
      }
      png.close();
    }

    tft.resetViewport();

    /*
        id = 0;
        while (id < 30) {
          moon.index = id;
          updateMoonDisplay(id);
          delay(100);
          id++;
        }

            moon.index = 1;
        updateMoonWarningDisplay();
        delay(500);
        moon.index = 0;
        updateMoonWarningDisplay();
        delay(500);
        moon.index = 29;
        updateMoonWarningDisplay();
        delay(500);
        moon.index = 2;
        updateMoonWarningDisplay();
        delay(500);

         */

    /*
    id = 0;
    currentWeather.isDay = 0;
    while (id < 4) {
      currentWeather.weatherCode = id;
      updateWeatherIcon();
      delay(beat_delay);
      id++;
    }

    id = 71;
    while (id < 79) {
      currentWeather.weatherCode = id;
      updateWeatherIcon();
      delay(beat_delay);
      id = id + 2;
    }

    id = 0;
    currentWeather.isDay = 1;
    while (id < 4) {
      currentWeather.weatherCode = id;
      updateWeatherIcon();
      delay(beat_delay);
      id++;
    }
*/
    // delay(3000);
    // Serial.println("-------------------------------------------");
    Serial.println();
  }
#endif
  // Serial.println("-------------------------------------------");
  // Serial.println();
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
        }
      }
    }
  } // Time set

  handleBacklight();

#ifndef NO_ANIM
  animate();
#endif
}

void animate() {

  uint32_t currentMillis = millis();
  static uint32_t previousMillis = 0;
  static uint8_t id;

  if (((uint32_t)(currentMillis - previousMillis) >= ANIMATION_MS) &&
      moon.fetchSuccess == 42) {

    updateMoonDisplay(id);
    id++;
    if (id == 30) {
      id = 0;
    }

    previousMillis = currentMillis;
  }
}

void drawStatic() {

  tft.drawFastHLine(0, TOP_LINE_Y, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, TOP_LINE_Y - 1, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, TOP_LINE_Y - 2, DISP_W, H_LINE_COLOR);

  tft.drawFastHLine(0, BTM_LINE_Y, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, BTM_LINE_Y + 1, DISP_W, H_LINE_COLOR);
  tft.drawFastHLine(0, BTM_LINE_Y + 2, DISP_W, H_LINE_COLOR);

  drawColon();
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

    if (djph) {
      tft.setTextColor(TFT_PINK, TFT_BLACK);
      snprintf(buffer, 50, " 8===o~~");
    }

    // snprintf(buffer, 50, "Fetching...");
    //  TODO:  Shove the caution icon in here or something like that
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

  tft.setViewport(VP_WEA_ICON_X, VP_WEA_ICON_Y, VP_WEA_ICON_W, VP_WEA_ICON_H);
  // tft.fillScreen(TFT_BLUE);

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

  tft.resetViewport();
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

void updateMoonDisplay(uint8_t index) {

  char imageName[80];

  xpos = VP_MOON_ICON_X;
  ypos = VP_MOON_ICON_Y;

  snprintf(imageName, 80, "/moon/small/%d.png", index);

  /*
    Serial.print("image filename: ");
    Serial.println(imageName);
   */

  int16_t rc =
      png.open(imageName, pngOpen, pngClose, pngRead, pngSeek, pngDraw);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    /*
        Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n",
                      png.getWidth(), png.getHeight(), png.getBpp(),
                      png.getPixelType());
     */
    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
    } else {
      rc = png.decode(NULL, 0);
      png.close();
    }
    tft.endWrite();
  }

  if (moon.fetchSuccess == FETCH_OK) {
    updateMoonWarningDisplay();
  }
}

void updateMoonWarningDisplay() {

  char imageName[40];

  Serial.print("moon warning: ");
  Serial.println(moon.index);

  switch (moon.index) {

  case 11:
  case 17:
  case 26:
  case 2:
    // Clear icon and return
    tft.fillRect(2, 10, 30, 30, TFT_BLACK);
    return;
    break;

  case 12:
  case 16:
  case 27:
  case 1:
    snprintf(imageName, 40, "/other/small/%s", "caution.png");
    break;

  case 13:
  case 15:
  case 28:
  case 0:
    snprintf(imageName, 40, "/other/small/%s", "warning.png");
    break;

  case 14:
  case 29:
    snprintf(imageName, 40, "/other/small/%s", "alert.png");
    break;

  default:
    return; // Nothing to do for the other days
  }

  xpos = MOON_WARNING_X;
  ypos = MOON_WARNING_Y;

  Serial.print("image filename: ");
  Serial.println(imageName);

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

void drawColon() {
  // The ':' in the middle
  tft.setTextColor(TIME_COLOR, TFT_BLACK);
  tft.setFreeFont(FONT_COLON);
  tft.drawChar(':', COLON_X_OFFSET, COLON_Y_OFFSET, GFXFF);
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
    // tft.fillScreen(TFT_BLACK);
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
  // Serial.printf("Attempting to open %s\n", filename);
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