#include "datasources.h"
#include "defines.h"
#include "display.h"

WiFiClientSecure client;
HTTPClient http;

uint32_t unixTime = 1904249932; // Debug

void handleDataSources();
void updateDataSources();

char fetchURL[BUF_SIZE]; // Buffer for working on JSON (used by both moon and weather)

////////////////////////////// Moon //////////////////////////////
JsonDocument moonInfo; // Allocate the JSON document
struct moonData moon;

////////////////////////////// Weather //////////////////////////////
JsonDocument weatherInfo; // Allocate the JSON document
struct weatherData currentWeather;

/////////////////////////////////////////////////////////////////////////////

void handleDataSources() {}

void updateDataSources() {
  updateMoonData();
  updateWeatherData();
}

////////////////////////////// Moon //////////////////////////////
void updateMoonData() {

  client.setInsecure();

  snprintf(fetchURL, BUF_SIZE, URL_BASE_MOON "%u", unixTime);

  /* Reply from API:
    [{"Error":0,"ErrorMsg":"success","TargetDate":"154451241564","Moon":["Planting
    Moon"],"Index":18,"Age":18.11856943237821582215474336408078670501708984375,"Phase":"Waning
    Gibbous","Distance":386011.900000000023283064365386962890625,"Illumination":0.88000000000000000444089209850062616169452667236328125,"AngularDiameter":0.5159378736629933737134479088126681745052337646484375,"DistanceToSun":151246162.938770711421966552734375,"SunAngularDiameter":0.527320148546759792651528186979703605175018310546875}]
  */

  // Serial.print("URL: ");
  // Serial.println(fetchURL);

  http.begin(client, fetchURL);
  http.GET();

  String fetchedJSON = http.getString();

  // Serial.print("JSON: ");
  // Serial.println(fetchedJSON);

  fetchedJSON = fetchedJSON.substring(1, fetchedJSON.length() - 1);

  deserializeJson(moonInfo, fetchedJSON); // Parse response

  http.end();

  moon.fetchSuccess = moonInfo["Error"]; // 0 = no errors

  if (!moon.fetchSuccess) {
    moon.index = moonInfo["Index"];
    moon.age = moonInfo["Age"];

    strcpy(moon.phase, moonInfo["Phase"]);
    strcpy(moon.name, moonInfo["Moon"][0]);

    // Print the values
    Serial.println(moon.phase);
    Serial.println(moon.name);
    Serial.println(moon.index);
    Serial.println(moon.age, 8);
    updateMoonDisplay();
  } else {
    Serial.print("Moon data fetch error: ");
    Serial.println(moon.fetchSuccess);
  }
  Serial.println();
}

////////////////////////////// Weather //////////////////////////////
void updateWeatherData() {

  snprintf(fetchURL, BUF_SIZE,
           URL_BASE_WEATHER "latitude=%.2f&longitude=%.2f&%s%s", LAT, LONG,
           "current=",
           "temperature_2m,relative_humidity_2m,apparent_temperature,"
           "precipitation,wind_speed_10m,wind_direction_10m");

  /* Reply from API:
    {"latitude":55.6875,"longitude":61.5,"generationtime_ms":0.014066696166992188,"utc_offset_seconds":0,"timezone":"GMT","timezone_abbreviation":"GMT","elevation":213.0,"current_units":{"time":"unixtime","interval":"seconds","temperature_2m":"°C"},"current":{"time":1705112100,"interval":900,"temperature_2m":-14.4}}
  */

  Serial.print("URL: ");
  Serial.println(fetchURL);

  client.setInsecure();

  http.begin(client, fetchURL);
  http.GET();

  String fetchedJSON = http.getString();

  Serial.print("JSON: ");
  Serial.println(fetchedJSON);

  deserializeJson(weatherInfo, fetchedJSON); // Parse response

  http.end();

  currentWeather.temp = weatherInfo["current"]["temperature_2m"];
  currentWeather.feels = weatherInfo["current"]["apparent_temperature"];
  currentWeather.windSpeed = weatherInfo["current"]["wind_speed_10m"];
  currentWeather.windDir = weatherInfo["current"]["wind_direction_10m"];
  currentWeather.humidity = weatherInfo["current"]["relative_humidity_2m"];

  Serial.println();
  Serial.print("Current Temperature: ");
  Serial.print(currentWeather.temp);
  Serial.println("°C");

  Serial.print("Apparent Temperature: ");
  Serial.print(currentWeather.feels);
  Serial.println("°C");

  Serial.print("Wind Speed: ");
  Serial.print(currentWeather.windSpeed);
  Serial.println("km/h");

  Serial.print("Wind Direction: ");
  Serial.print(currentWeather.windDir);
  Serial.println("°");

  Serial.print("Relative Humidity: ");
  Serial.print(currentWeather.humidity);
  Serial.println("%");
  Serial.println();
}