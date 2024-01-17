#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <LittleFS.h>
#include <PNGdec.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <Timezone.h>

#include "fonts/NotoSans_Regular12pt7b.h"
#include "fonts/NotoSans_Regular14pt7b.h"
#include "fonts/NotoSans_Regular16pt7b.h"
#include "fonts/NotoSans_Regular20pt7b.h"
#include "fonts/NotoSans_Regular42pt7b.h"
#include "fonts/NotoSans_Regular70pt7b.h"

#define FileSys LittleFS

#define DISP_W 320
#define DISP_H 240

#define TFT_BACKLIGHT 5
#define PWM1_CH 0
#define PWM1_RES 10
#define PWM1_FREQ 1000
#define CDS_PIN 34
#define CDS_AVG_COUNT 50
#define CDS_READ_RATE 50
#define PWM_MINIMUM 950 // PWM is inverted!

#define MOON_ANIM_FR 125
#define ANIMATION_START_DELAY (10 * 1000);

#define ANIM_FRAME_COUNT 42
#define WEATHER_ANIM_FR 75
#define WEATHER_ANIM_ICON_SIZE 20
#define ANIM_ORIGIN_X (VP_WEA_ICON_X - WEATHER_ANIM_ICON_SIZE)
#define ANIM_ORIGIN_Y (VP_WEA_ICON_H - WEATHER_ANIM_ICON_SIZE - 10)

// Static items
#define COLON_X_OFFSET 150
#define COLON_Y_OFFSET (VP_TIME_Y + 80)
#define TOP_LINE_Y (VP_TIME_Y - 13)
#define BTM_LINE_Y (VP_TIME_Y + 125)
#define H_LINE_COLOR TFT_DARKCYAN

// Viewports
#define VP_TIME_X 0
#define VP_TIME_Y 68
#define VP_TIME_W 75
#define VP_TIME_H 110
#define TIME_COLOR TFT_GOLD

#define VP_DATE_Y (BTM_LINE_Y + 16)
#define VP_DATE_W 320
#define VP_DATE_H 60
#define DATE_COLOR TFT_SKYBLUE

#define VP_MOON_ICON_X 35
#define VP_MOON_ICON_Y 8
#define VP_MOON_ICON_W 40
#define VP_MOON_ICON_H 38
#define MOON_WARNING_X 5
#define MOON_WARNING_Y 16

#define VP_WEA_X 200
#define VP_WEA_Y 15
#define VP_WEA_W 120
#define VP_WEA_H 28
#define VP_WEA_ICON_X (VP_WEA_X - 65)
#define VP_WEA_ICON_Y 0
#define VP_WEA_ICON_H 50
#define VP_WEA_ICON_W 65
#define WEA_ICON_X 0
#define WEA_ICON_Y 0
#define WEATHER_COLOR TFT_SKYBLUE

// The custom font file attached to this sketch must be included
#define GFXFF 1 // Stock font and GFXFF reference handle

void setupDisplay();
void handleDisplay();
void animate();
void handleBacklight();
void drawStatic();
void updateWeatherDisplay();
void updateWeatherIcon(bool tiny = false);
void animateWeather();
void updateMoonDisplay(uint8_t index);
void updateMoonWarningDisplay();
void updateDateString();
void drawColon();
void updateTimeDisplay(); // Updates both hours and minutes
void updateHoursDisplay();
void updateMinutesDisplay();
void updateDateString();
void updateDateDisplay();

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDrawImage(PNGDRAW *pDraw);
void pngDrawAnim(PNGDRAW *pDraw);
// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.
void *pngOpen(const char *filename, int32_t *size);
void pngClose(void *handle);
int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length);
int32_t pngSeek(PNGFILE *page, int32_t position);
#endif