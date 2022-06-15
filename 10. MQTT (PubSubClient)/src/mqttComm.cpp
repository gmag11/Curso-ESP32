#include "mqttComm.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "wificonfig.h"
#include <QuickDebug.h>
#include "esp_log.h"
#include "esp32-hal-log.h"

static const char* MQTT_TAG = "MQTT";

const char* mqtt_server = "192.168.5.120";
const char* ledTopic = "led/set";
const char* ledStateTopic = "led/state";
const char* buttonTopic = "led/set";

WiFiClient espClient;
PubSubClient mqttClient (espClient);

extern bool ledChanged;
extern bool ledOn;

void callback (char* topic, byte* payload, unsigned int length) {
    DEBUG_INFO (MQTT_TAG, "Message arrived: %s : [% .*s] \n", topic, length, payload);

    if (!strcmp (topic, ledTopic) && length == 1) {
        if (payload[0] == '1') {
            if (!ledOn) {
                ledOn = true;
                ledChanged = true;
                DEBUG_INFO (MQTT_TAG, "Led on");
            }
        } else {
            if (ledOn) {
                ledOn = false;
                ledChanged = true;
                DEBUG_INFO (MQTT_TAG, "Led off");
            }
        }
    } else if (!strcmp (topic, ledStateTopic) && length == 1) {
        if (payload[0] == '?') {
            DEBUG_INFO (MQTT_TAG, "Led state: %d", ledOn);
            mqttClient.publish (ledStateTopic, ledOn ? "1" : "0");
        }
    }
}

void reconnect () {
  // Loop until we're reconnected
    while (!mqttClient.connected ()) {
        DEBUG_INFO (MQTT_TAG, "Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String ((int32_t)(ESP.getEfuseMac () & 0xFFFFFFFF), HEX);
        clientId += String ((int32_t)(ESP.getEfuseMac () >> 32), HEX);
        // Attempt to connect
        if (mqttClient.connect (clientId.c_str (), MQTT_USER, MQTT_PASSWORD)) {
            DEBUG_INFO (MQTT_TAG, "connected");
            // Once connected, publish an announcement...
            mqttClient.publish ("led/client", clientId.c_str ());
            // ... and resubscribe
            mqttClient.subscribe (ledTopic);
            mqttClient.subscribe (ledStateTopic);
        } else {
            DEBUG_INFO (MQTT_TAG, "failed, rc=%d try again in 5 seconds", mqttClient.state ());
            // Wait 5 seconds before retrying
            delay (5000);
        }
    }
}

void initMqttComm () {
    mqttClient.setServer (mqtt_server, 1883);
    mqttClient.setCallback (callback);
    esp_log_level_set (MQTT_TAG, ESP_LOG_ERROR);
}

void mqttUpdate () {
    if (!mqttClient.connected ()) {
        reconnect ();
    }
    mqttClient.loop ();
}

void sendButtonMqtt (bool state) {
    mqttClient.publish (buttonTopic, state ? "1" : "0");
    DEBUG_INFO (MQTT_TAG, "Sent button topic");
}