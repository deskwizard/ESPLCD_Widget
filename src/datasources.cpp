
/*

     Remote = MQTT local
     Local = onboard
     External = internet stuff

     Fetch internet stuff on SBC instead ??

*/
#include "datasources.h"
#include "defines.h"
#include "display.h"
#include "network.h"
#include <HTU2xD_SHT2x_Si70xx.h>
#include <PubSubClient.h>

HTU2xD_SHT2x_SI70xx ht2x(HTU2xD_SENSOR, HUMD_11BIT_TEMP_11BIT); // sensor type, resolution

WiFiClientSecure wifiClientSecure;
HTTPClient httpClient;

///////////////////////////////////////////////////////////////////////////////////////////////////////
const char *mqtt_server = "192.168.2.22";
uint16_t mqtt_port = 1883;

// Publishing
const char *deviceIDBase = "BME280-";
char deviceID[13] = ""; // Filled later with MAC
char mqttTopic[] = "bedroom/sensor";
uint16_t publishInterval = 15000;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
void callback(char *topic, byte *payload, unsigned int length) {};
void publish();
///////////////////////////////////////////////////////////////////////////////////////////////////////

// uint32_t unixTime = 1904249932; // Debug

void handleDataSources();
void updateDataSources();

char fetchURL[BUF_SIZE]; // JSON Buffer

uint32_t dataUpdateDelay = (MINUTES_TO_MS * 1);

struct sensorData localSensor;
struct moonData moon;
struct weatherData currentWeather;
JsonDocument moonInfo;    // Allocate the Moon JSON document
JsonDocument weatherInfo; // Allocate the Weather JSON document

/////////////////////////////////////////////////////////////////////////////

void connect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect...
    if (client.connect(deviceID))
    {
      Serial.println("connected");

      // client.publish(mqttTopicT, "hello world");

      // ... and resubscribe
      // client.subscribe(inTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupDataSources()
{
  if (ht2x.begin() != true) // reset sensor, set heater off, set resolution, check power (sensor doesn't operate correctly if VDD < +2.25v)
  {
    Serial.println("-!- Local Sensor FAILURE -!-");
  }
  else
  {
    Serial.println("Local Sensor Connected");
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void handleDataSources()
{

  if (!client.connected() && WiFi.status() == WL_CONNECTED)
  {
    connect();
  }
  client.loop();

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

  updateLocalSensorData();
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

void updateLocalSensorData()
{
  uint32_t currentMillis = millis();
  static uint32_t previousMillis = 0;

  if ((uint32_t)(currentMillis - previousMillis) >= 15000)
  {

    Serial.println("----------- Sensor Data -----------");

    float htValue = ht2x.readTemperature();

    if (htValue != HTU2XD_SHT2X_SI70XX_ERROR) // HTU2XD_SHT2X_SI70XX_ERROR = 255
    {
      // We already have the temperature in 'htValue'
      localSensor.temperature = htValue;

      htValue = ht2x.getCompensatedHumidity(htValue);
      localSensor.humidity = uint8_t(htValue + 0.5f); // Round up

      localSensor.fetchSuccess = 0;

      publish();
    }
    else
    {
      localSensor.fetchSuccess = FETCH_FAIL;
    }

    if (localSensor.fetchSuccess == 0)
    {
      Serial.print("Temperature...: ");
      Serial.print(localSensor.temperature + 0.05f, 1);
      Serial.println("°C +-0.3°");
      Serial.print("Humidity......: ");
      Serial.print(localSensor.humidity);
      Serial.println("% +-2%");
    }
    Serial.println();

    previousMillis = currentMillis;
  }
}

void publish()
{
  char publishMessage[75];


  //    Serial.print("T: ");
  //    Serial.print(temperature);
  //    Serial.print(" H: ");
  //    Serial.print(humidity);
  snprintf(publishMessage, 75, "{\"temperature\": %.1f, \"humidity\": %d}", localSensor.temperature + 0.05f, localSensor.humidity);

  Serial.print("Publish message: ");
  Serial.println(publishMessage);

  client.publish(mqttTopic, publishMessage,
                 false); // true = persistant, will keep last msg
}

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
           URL_BASE_WEATHER "latitude=%.8f&longitude=%.2f&%s%s", LAT, LONG,
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