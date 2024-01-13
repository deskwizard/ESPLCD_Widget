#ifndef _DISPLAY_H_
#define _DISPLAY_H_

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
#include "fonts/MoonPhases20pt7b.h"

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

// Static items
#define COLON_X_OFFSET 150
#define COLON_Y_OFFSET (VP_TIME_Y + 80)
#define TOP_LINE_Y (VP_TIME_Y - 4)
#define BTM_LINE_Y (VP_TIME_Y + 125)

// Viewports
#define VP_TIME_X 0
#define VP_TIME_Y 65
#define VP_TIME_W 75
#define VP_TIME_H 110

#define VP_DATE_Y (BTM_LINE_Y + 16)
#define VP_DATE_W 320
#define VP_DATE_H 60

#define VP_MOON_ICON_X 5
#define VP_MOON_ICON_Y 5
#define VP_MOON_ICON_W 40
#define VP_MOON_ICON_H 38

#define VP_WEA_X 120
#define VP_WEA_Y 5
#define VP_WEA_W 200
#define VP_WEA_H 38

// The custom font file attached to this sketch must be included
#define GFXFF 1 // Stock font and GFXFF reference handle

void setupDisplay();
void handleDisplay();
void handleBacklight();
void drawStatic();
void updateWeatherDisplay();
void updateMoonDisplay();
void updateDateString();
void updateTimeDisplay(); // Updates both hours and minutes
void updateHoursDisplay();
void updateMinutesDisplay();
void updateDateString();
void updateDateDisplay();

void neverGive();

#endif