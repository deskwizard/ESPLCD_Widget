#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <LittleFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
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
#define PWM_MINIMUM 1000 // PWM is inverted!

// Static items
#define COLON_X_OFFSET 150
#define COLON_Y_OFFSET (VP_TIME_Y + 80)
#define TOP_LINE_Y (VP_TIME_Y - 13)
#define BTM_LINE_Y (VP_TIME_Y + 125)
#define H_LINE_COLOR TFT_OLIVE

// Viewports
#define VP_TIME_X 0
#define VP_TIME_Y 68
#define VP_TIME_W 75
#define VP_TIME_H 110
#define TIME_COLOR TFT_ORANGE

#define VP_DATE_Y (BTM_LINE_Y + 16)
#define VP_DATE_W 320
#define VP_DATE_H 60
#define DATE_COLOR TFT_DARKCYAN

#define VP_MOON_ICON_X 38
#define VP_MOON_ICON_Y 8
#define VP_MOON_ICON_W 40
#define VP_MOON_ICON_H 38

#define VP_WEA_X 190
#define VP_WEA_Y 15
#define VP_WEA_W 120
#define VP_WEA_H 20
#define WEATHER_COLOR TFT_DARKCYAN
#define WEA_ICON_X (VP_WEA_X - 55);
#define WEA_ICON_Y 0

// The custom font file attached to this sketch must be included
#define GFXFF 1 // Stock font and GFXFF reference handle

const uint8_t moonMap[29] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                             'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                             'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B'};

void setupDisplay();
void handleDisplay();
void handleBacklight();
void drawStatic();
void updateWeatherDisplay();
void updateWeatherIcon();
void updateMoonDisplay();
void updateDateString();
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
void pngDraw(PNGDRAW *pDraw);

// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.
void *pngOpen(const char *filename, int32_t *size);
void pngClose(void *handle);
int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length);
int32_t pngSeek(PNGFILE *page, int32_t position);
#endif