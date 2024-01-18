// https://www.codeproject.com/Articles/5336116/Img2Cpp-Create-Cplusplus-Headers-for-Embedding-Ima
// https://notisrac.github.io/FileToCArray/

/******** WARNING: INCLUDE ORDER MATTERS !!! ********/
#include "display.h"
#include "datasources.h"
#include "defines.h"

File pngfile;
PNG png;
// Image coordinates used by pngDrawImage when drawing
int16_t xpos = 0;
int16_t ypos = 0;

TFT_eSPI tft = TFT_eSPI();

extern time_t localTime;
extern Timezone myTZ;
char dateString[50];

#define beat_delay 400 // Debug
bool djph = false;     // Debug

uint16_t animXPos = ANIM_ORIGIN_X;
uint16_t animYPos = ANIM_ORIGIN_Y;

uint8_t timeDigitsOffset = 0;

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
    handleBacklight();
  }

  tft.begin();
  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  // tft.fillScreen(TFT_PURPLE);

  drawStatic();
  updateMoonDisplay(29); ////////////////////////////////////////////////////
  // updateWeatherDisplay();
  updateTimeDisplay();
  updateDateString();
  updateDateDisplay();
  updateWeatherIcon(true); // true = show center animation icon

#ifdef TEST_DISPLAY
  uint8_t id = 0;

  updateWeatherIcon(true); // true = show center animation icon

  while (1) {

    animate();

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
    // Serial.println();
  }
#endif
}

void handleDisplay() {

  static uint8_t previousMinute = 0;

  if (timeStatus() != timeNotSet) {

    localTime = myTZ.toLocal(now());

    if (minute(localTime) != previousMinute) {

      serialClockDisplay();

      // updateMinutesDisplay();

      previousMinute = minute(localTime);

      if (minute(localTime) == 0) {

        updateHoursDisplay();

        if (hour(localTime) == 0) {
          updateDateString();
          updateDateDisplay();
        }
      }

      updateMinutesDisplay();
    }
  } // Time set

  handleBacklight();

#ifndef NO_ANIM
  animate();
#endif
}

void animate() {

  uint32_t currentMillis = millis();
  static uint32_t previousMoonMillis = 0;
  static uint32_t previousWeatherMillis = 0;
  static uint8_t id;

  if (((uint32_t)(currentMillis - previousWeatherMillis) >= WEATHER_ANIM_FR) &&
      currentWeather.fetchSuccess == 42) {
    animateWeather();
    previousWeatherMillis = currentMillis;
  }

  currentMillis = millis();
  if (((uint32_t)(currentMillis - previousMoonMillis) >= MOON_ANIM_FR) &&
      moon.fetchSuccess == 42) {

    updateMoonDisplay(id);
    id++;
    if (id == 30) {
      id = 0;
    }

    previousMoonMillis = currentMillis;
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

void updateWeatherIcon(bool tiny) {

  char imageFilename[80];

  if (tiny) {
    Serial.println("tiny");
    snprintf(imageFilename, 80, "/weather/small/anim_img.png");
    // Adjust location of the weather animation center image here
    // It needs adjustment when the image size changes (duh...).
    xpos = 7;
    ypos = 26;
  } else {
    snprintf(imageFilename, 80, "/weather/small/%d/%d.png",
             currentWeather.isDay, currentWeather.weatherCode);
    xpos = WEA_ICON_X;
    ypos = WEA_ICON_Y;
  }

  Serial.print("image filename: ");
  Serial.println(imageFilename);

  tft.setViewport(VP_WEA_ICON_X, VP_WEA_ICON_Y, VP_WEA_ICON_W, VP_WEA_ICON_H);
  // tft.fillScreen(TFT_BLUE);
  tft.fillScreen(TFT_BLACK);

  int16_t rc = png.open(imageFilename, pngOpen, pngClose, pngRead, pngSeek,
                        pngDrawImage);

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

void animateWeather() {

  // Currently has 42 frames per cycle (either moon or sun)
  static uint8_t currentFrame = 0;
  static bool iconSunOrMoon = true; // Sun = true, moon = false
  char imageFilename[30];

  if (currentFrame == 0) {
    animXPos = ANIM_ORIGIN_X;
    animYPos = ANIM_ORIGIN_Y;
  }

  // Load image
  snprintf(imageFilename, 30, "/weather/small/%d.png", iconSunOrMoon);

  int16_t rc =
      png.open(imageFilename, pngOpen, pngClose, pngRead, pngSeek, pngDrawAnim);

  // Enter viewport (false = keep coordinates on top left of the screen,
  //                 not of the viewport)
  tft.setViewport(VP_WEA_ICON_X, VP_WEA_ICON_Y, VP_WEA_ICON_W, VP_WEA_ICON_H,
                  false);
  // tft.fillScreen(TFT_PURPLE); // Will erase the center icon, beware.

  animXPos = animXPos + 2;

  // This could be reworked
  if (currentFrame < 20 || currentFrame > 22) {
    if (animXPos <= (ANIM_X_OFFSET - WEATHER_ANIM_ICON_SIZE)) {
      animYPos--;
    } else {
      animYPos++;
    }
  }

  tft.startWrite();
  rc = png.decode(NULL, 0);
  tft.endWrite();

  currentFrame++;

  if (currentFrame >= ANIM_FRAME_COUNT) {
    iconSunOrMoon = !iconSunOrMoon; // Toggle sun/moon image
    currentFrame = 0;
  }

  png.close();

  tft.resetViewport();

  /*
    Serial.print("currentFrame: ");
    Serial.println(currentFrame);
  */

  // start over
  /*
    djph = true;
    updateMoonDisplay(30);
    updateWeatherDisplay();
   */
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
    // Serial.println();

    // Check for limits before writing
    ledcWrite(PWM1_CH, outputPWMValue);

    lastReadReadMillis = currentMillis;
  }
}

void updateMoonDisplay(uint8_t index) {

  char imageFilename[80];

  xpos = VP_MOON_ICON_X;
  ypos = VP_MOON_ICON_Y;

  snprintf(imageFilename, 80, "/moon/small/%d.png", index);

  int16_t rc = png.open(imageFilename, pngOpen, pngClose, pngRead, pngSeek,
                        pngDrawImage);

  if (rc == PNG_SUCCESS) {
    tft.startWrite();

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

  char imageFilename[40];

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
    snprintf(imageFilename, 40, "/other/small/%s", "caution.png");
    break;

  case 13:
  case 15:
  case 28:
  case 0:
    snprintf(imageFilename, 40, "/other/small/%s", "warning.png");
    break;

  case 14:
  case 29:
    snprintf(imageFilename, 40, "/other/small/%s", "alert.png");
    break;

  default:
    return; // Nothing to do for the other days
  }

  xpos = MOON_WARNING_X;
  ypos = MOON_WARNING_Y;

  Serial.print("image filename: ");
  Serial.println(imageFilename);

  int16_t rc = png.open(imageFilename, pngOpen, pngClose, pngRead, pngSeek,
                        pngDrawImage);

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

  return;
  // The ':' in the middle
  tft.setTextColor(TIME_COLOR, TFT_BLACK);
  tft.setFreeFont(FONT_COLON);
  tft.drawChar(':', COLON_X_OFFSET - timeDigitsOffset, COLON_Y_OFFSET, GFXFF);
}

void updateTimeDisplay() {
  updateHoursDisplay();
  drawColon();
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
  //  tft.drawNumber(tens, 0, 0, GFXFF);

  if (tens != 0) {
    tft.drawNumber(tens, 0, 0, GFXFF);
    timeDigitsOffset = 0;
  } else {
    tft.fillScreen(TFT_NAVY);
    timeDigitsOffset = VP_TIME_W / 2;
  }

  /*
    if (tens >= 10) {
      tft.drawNumber(tens, 0, 0, GFXFF);
      timeDigitsOffset = 0;
      //tft.fillScreen(TFT_BLACK);
    } //
    else {
      // tft.fillScreen(TFT_BLACK);
      //timeDigitsOffset = VP_TIME_W / 2;
    }
    */
  tft.resetViewport();

  tft.setViewport(VP_TIME_W - timeDigitsOffset, VP_TIME_Y, VP_TIME_W,
                  VP_TIME_H);
  tft.drawNumber(ones, 0, 0, GFXFF);
  tft.resetViewport();
}

void updateMinutesDisplay() {
  Serial.println("updateMinutesDisplay() called.");
  Serial.print("offset: ");
  Serial.println(timeDigitsOffset);

  localTime = myTZ.toLocal(now());

  uint8_t tens = minute(localTime);
  uint8_t ones = tens % 10;
  tens = tens / 10;
  tens = tens % 10;

  tft.setFreeFont(FONT_TIME);

  tft.setTextColor(TIME_COLOR, TFT_BLACK);

  tft.setViewport((DISP_W - (VP_TIME_W * 2)) - timeDigitsOffset, VP_TIME_Y,
                  VP_TIME_W, VP_TIME_H);
  // tft.setViewport((DISP_W - (VP_TIME_W * 2)), VP_TIME_Y, VP_TIME_W,
  // VP_TIME_H);
  tft.drawNumber(tens, 0, 0, GFXFF);
  tft.resetViewport();

  tft.setViewport((DISP_W - VP_TIME_W) - timeDigitsOffset, VP_TIME_Y, VP_TIME_W,
                  VP_TIME_H);
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
void pngDrawImage(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void pngDrawAnim(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(animXPos, animYPos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
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