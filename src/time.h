#ifndef _TIME_H_
#define _TIME_H_

#include <TimeLib.h>    // https://www.pjrc.com/teensy/td_libs_Time.html
#include <Timezone.h>   // https://github.com/JChristensen/Timezone
#include <DS3231RTC.h>  // Mine

#define MINUTES_TO_MS 60000L

#define DEFAULT_TIME  1357041600 // Jan 1 2013
#define RTC_UPDATE_INTERVAL 55 // in seconds

void setupTime();
void handleNTP();
void serialClockDisplay();

#endif
