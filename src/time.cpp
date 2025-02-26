
// On Linux, you can use "date +T%s\n > /dev/ttyUSB0" for UTC timezone

#include "time.h"
#include "defines.h"

extern bool NTPState;

time_t updateTime();

// US Eastern Time Zone (New York, Detroit, Toronto, Montreal, etc...)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240}; // UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};  // UTC - 5 hours
TimeChangeRule *tcr; // Pointer to the time change rule, use to get TZ abbrev
Timezone myTZ(myDST, mySTD);
time_t localTime;

// Debug
bool forceNTPFail = false;
bool forceBadRTC = false;
bool serialClockEnabled = false;

void setupTime() {

  // Set time library update function and update interval
  setSyncProvider(updateTime);
  setSyncInterval(RTC_UPDATE_INTERVAL);

  // Serial.print("DST?: ");
  // Serial.println(myTZ.locIsDST(now()));
}

time_t updateTime() {

  Serial.print("Trying RTC... ");

  time_t rtcTime = RTC.get();

  if (forceBadRTC) {
    Serial.println("RTC invalidated.");
    return 0;
  }

  if (rtcTime <=
      DEFAULT_TIME) { // TODO: Add < bad time (2106 something like that) too.
    Serial.println("RTC time is invalid.");
    return 0;
  } else {
    Serial.println("RTC time is Valid, time updated.");

    return rtcTime + 1;
  }
}

// Print an integer in "00" format (with leading zero).
// Input value assumed to be between 0 and 99.
void sPrintI00(int val) {
  if (val < 10)
    Serial.print('0');
  Serial.print(val, DEC);
  return;
}

// Print an integer in ":00" format (with leading zero).
// Input value assumed to be between 0 and 99.
void sPrintDigits(int val) {
  Serial.print(':');
  if (val < 10)
    Serial.print('0');
  Serial.print(val, DEC);
}

// Function to print time
void printTime(time_t t) {
  sPrintI00(hour(t));
  sPrintDigits(minute(t));
  sPrintDigits(second(t));
  Serial.print(' ');
  Serial.print(dayShortStr(weekday(t)));
  Serial.print(' ');
  Serial.print(monthShortStr(month(t)));
  Serial.print(' ');
  sPrintI00(day(t));
  Serial.print(' ');
  Serial.println(year(t));
}

void printDigits(int digits) {

  // utility for digital clock display: prints preceding colon and leading 0

  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void serialClockDisplay() {

  localTime = myTZ.toLocal(now());

  // digital clock display of the time
  Serial.print(hour(localTime));
  Serial.print(":");
  printDigits(minute(localTime));
  Serial.print(":");
  printDigits(second(localTime));
  Serial.print(" ");
  printDigits(day(localTime));
  Serial.print("/");
  printDigits(month(localTime));
  Serial.print("/");
  Serial.print(year(localTime));
  Serial.println();
}
