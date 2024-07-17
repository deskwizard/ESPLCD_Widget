#include "mqtt.h"
#include "network.h"
#include <PubSubClient.h>
#include "sensor.h"

const char *mqtt_server = "192.168.2.22";
uint16_t mqtt_port = 1883;

// Publishing
const char *deviceIDBase = "BME280-";
char deviceID[13] = ""; // Filled later with MAC
char mqttTopic[] = "bedroom/sensor";
uint16_t publishInterval = 15000;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
void callbackMQTT(char *topic, byte *payload, unsigned int length) {};

void setupMQTT()
{
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callbackMQTT);
}

void connectMQTT()
{
    // Loop until we're reconnected
    if (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        // Attempt to connect...
        if (client.connect(deviceID))
        {
            Serial.println(" Connected!");

            // client.publish(mqttTopicT, "hello world");

            // ... and resubscribe
            // client.subscribe(inTopic);
        }
        else
        {
            Serial.print(" Connection failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again later");
            // Wait 5 seconds before retrying
            // delay(5000);
        }
    }
}

void publishMQTT()
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

void handleMQTT()
{

    if (!client.connected() && WiFi.status() == WL_CONNECTED)
    {
        connectMQTT();
    }

    client.loop();
}
