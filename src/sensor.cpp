#include "defines.h"
#include "sensor.h"
#include "network/mqtt.h"
#include <HTU2xD_SHT2x_Si70xx.h>
HTU2xD_SHT2x_SI70xx ht2x(HTU2xD_SENSOR, HUMD_11BIT_TEMP_11BIT); // sensor type, resolution
struct sensorData localSensor;

void setupLocalSensor()
{

    if (ht2x.begin() != true) // reset sensor, set heater off, set resolution, check power (sensor doesn't operate correctly if VDD < +2.25v)
    {
        Serial.println("-!- Local Sensor FAILURE -!-");
    }
    else
    {
        Serial.println("Local Sensor Connected");
    }
}

void handleLocalSensor()
{

    uint32_t currentMillis = millis();
    static uint32_t previousMillis = 0;

    if ((uint32_t)(currentMillis - previousMillis) >= 15000)
    {

        Serial.println();
        Serial.println("----------- Sensor Data -----------");

        float htValue = ht2x.readTemperature();

        if (htValue != HTU2XD_SHT2X_SI70XX_ERROR) // HTU2XD_SHT2X_SI70XX_ERROR = 255
        {
            // We already have the temperature in 'htValue'
            localSensor.temperature = htValue;

            htValue = ht2x.getCompensatedHumidity(htValue);
            localSensor.humidity = uint8_t(htValue + 0.5f); // Round up

            localSensor.fetchSuccess = 0;

            publishMQTT();
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
