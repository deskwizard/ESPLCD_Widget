
/*

     Remote = MQTT local
     Local = onboard
     External = internet stuff

     Fetch internet stuff on SBC instead ??

*/
#include "datasources.h"
#include "defines.h"
#include "display.h"
#include "network/network.h"

WiFiClientSecure wifiClientSecure;
HTTPClient httpClient;

void handleDataSources();
void updateDataSources();

char fetchURL[BUF_SIZE]; // JSON Buffer

// uint32_t dataUpdateDelay = (MINUTES_TO_MS * 1);
uint32_t dataUpdateDelay = ANIMATION_START_DELAY; // Wait so it animates a bit

//struct sensorData localSensor;
struct moonData moon;
struct weatherData currentWeather;
JsonDocument moonInfo;    // Allocate the Moon JSON document
JsonDocument weatherInfo; // Allocate the Weather JSON document


void handleDataSources()
{

  // if "it's time" and wifi is connected
  uint32_t currentMillis = millis();
  static uint32_t dataUpdateMillis = currentMillis;
  // static uint32_t dataUpdateDelay = (MINUTES_TO_MS * 1);

  if ((uint32_t)(currentMillis - dataUpdateMillis) >= dataUpdateDelay)
  {

    Serial.println();
    Serial.print("-!- Remote Data Update Timeout at ");
    serialClockDisplay();

    if (WiFi.status() == WL_CONNECTED)
    {
      updateMoonData();
      updateWeatherData();

      dataUpdateDelay = DATA_UPDATE_INTERVAL;
    }
    else
    {
      Serial.println("No network");
      dataUpdateDelay = DATA_RETRY_INTERVAL;
    }
    /*
      else {
        Serial.println("---- WiFi disconnected, attempting connection...");
        connectWiFi();
      }
    */
    dataUpdateMillis = currentMillis;
  }

}
/*
void updateDataSources() {
  uint32_t savedMillis = millis();
  updateMoonData();
  Serial.print("Moon fetch delta: ");
  Serial.println(millis() - savedMillis);

  savedMillis = millis();
  updateWeatherData();
  Serial.print("Weather fetch delta: ");
  Serial.println(millis() - savedMillis);
}
*/


//////////////////////////////// Moon ///////////////////////////////
void updateMoonData()
{

  // updateMoonDisplay(29);

  wifiClientSecure.setInsecure();

  snprintf(fetchURL, BUF_SIZE, URL_BASE_MOON "%u", now());

  /* Reply from API:
    [{"Error":0,"ErrorMsg":"success","TargetDate":"154451241564","Moon":["Planting
    Moon"],"Index":18,"Age":18.11856943237821582215474336408078670501708984375,"Phase":"Waning
    Gibbous","Distance":386011.900000000023283064365386962890625,"Illumination":0.88000000000000000444089209850062616169452667236328125,"AngularDiameter":0.5159378736629933737134479088126681745052337646484375,"DistanceToSun":151246162.938770711421966552734375,"SunAngularDiameter":0.527320148546759792651528186979703605175018310546875}]
  */

  // Serial.print("URL: ");
  // Serial.println(fetchURL);

  httpClient.begin(wifiClientSecure, fetchURL);
  httpClient.GET();

  String fetchedJSON = httpClient.getString();

  // Serial.print("Moon JSON: ");
  // Serial.println(fetchedJSON);

  fetchedJSON = fetchedJSON.substring(1, fetchedJSON.length() - 1);

  deserializeJson(moonInfo, fetchedJSON); // Parse response

  httpClient.end();

  moon.fetchSuccess = moonInfo["Error"]; // 0 = no errors

  Serial.println();
  Serial.println("----------- Moon Data -----------");

  if (!moon.fetchSuccess)
  {

    strcpy(moon.name, moonInfo["Moon"][0]);
    strcpy(moon.phase, moonInfo["Phase"]);
    moon.age = moonInfo["Age"];
    moon.illumination = moonInfo["Illumination"];
    moon.index = moonInfo["Index"];

    // Print the values
    Serial.print(moon.phase);
    Serial.print(" (");
    Serial.print(moon.name);
    Serial.println(")");
    Serial.print(moon.illumination);
    Serial.println("% Illumination");
    Serial.print(moon.age, 2);
    Serial.println(" days old");
    Serial.print("Image index: ");
    Serial.println(moon.index);

    updateMoonDisplay(moon.index);
  } //
  else
  {
    Serial.print("Moon data fetch error: ");
    Serial.println(moon.fetchSuccess);
  }
}

////////////////////////////// Weather //////////////////////////////
void updateWeatherData()
{

  snprintf(fetchURL, BUF_SIZE,
           URL_BASE_WEATHER "latitude=%.8f&longitude=%.2f&%s%s", WEATHER_LAT, WEATHER_LONG,
           "current=",
           "temperature_2m,relative_humidity_2m,apparent_temperature,"
           "precipitation,wind_speed_10m,wind_direction_10m,cloud_cover,"
           "weather_code,is_day");

  /* Reply from API:
    {"latitude":55.6875,"longitude":61.5,"generationtime_ms":0.014066696166992188,"utc_offset_seconds":0,"timezone":"GMT","timezone_abbreviation":"GMT","elevation":213.0,"current_units":{"time":"unixtime","interval":"seconds","temperature_2m":"°C"},"current":{"time":1705112100,"interval":900,"temperature_2m":-14.4}}
  */

  // Serial.print("URL: ");
  // Serial.println(fetchURL);

  wifiClientSecure.setInsecure();

  httpClient.begin(wifiClientSecure, fetchURL);
  httpClient.GET();

  String fetchedJSON = httpClient.getString();

  // Serial.print("JSON: ");
  // Serial.println(fetchedJSON);

  deserializeJson(weatherInfo, fetchedJSON); // Parse response

  httpClient.end();

  currentWeather.fetchSuccess = weatherInfo["error"]; // 0 = no errors

  Serial.println();
  Serial.println("----------- Weather Data -----------");

  if (currentWeather.fetchSuccess == FETCH_OK)
  {

    currentWeather.temperature = weatherInfo["current"]["temperature_2m"];
    currentWeather.feels = weatherInfo["current"]["apparent_temperature"];
    currentWeather.precipitation = weatherInfo["current"]["precipitation"];
    currentWeather.cloudCover = weatherInfo["current"]["cloud_cover"];
    currentWeather.windSpeed = weatherInfo["current"]["wind_speed_10m"];
    currentWeather.windDir = weatherInfo["current"]["wind_direction_10m"];
    currentWeather.humidity = weatherInfo["current"]["relative_humidity_2m"];
    currentWeather.weatherCode = weatherInfo["current"]["weather_code"];
    currentWeather.isDay = weatherInfo["current"]["is_day"];

    Serial.print("Weather Code: ");
    Serial.print(currentWeather.weatherCode);
    Serial.println();

    Serial.print("Current Temperature: ");
    Serial.print(currentWeather.temperature);
    Serial.println("°C");

    Serial.print("Apparent Temperature: ");
    Serial.print(currentWeather.feels);
    Serial.println("°C");

    Serial.print("Precipitation: ");
    Serial.print(currentWeather.precipitation);
    Serial.println("mm");

    Serial.print("Cloud Cover: ");
    Serial.print(currentWeather.cloudCover);
    Serial.println("%");

    Serial.print("Wind Speed: ");
    Serial.print(currentWeather.windSpeed);
    Serial.println("km/h");

    Serial.print("Wind Direction: ");
    Serial.print(currentWeather.windDir);
    Serial.println("°");

    Serial.print("Day time?: ");
    Serial.println(currentWeather.isDay);

    Serial.print("Relative Humidity: ");
    Serial.print(currentWeather.humidity);
    Serial.println("%");
    Serial.println();

    updateWeatherIcon();
    updateCurrentWeatherDisplay();

  } //
  else
  {
    Serial.println("Weather fetch error: ");
    Serial.println(currentWeather.fetchSuccess);
  }
}