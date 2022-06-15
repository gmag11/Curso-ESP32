#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <QuickDebug.h>

#include <M5StickCPlus.h>
#include <list>
#include <ArduinoJson.h>
#include "webServer.h"
#include "configStorage.h"
#include "display.h"
#include "timeSync.h"
#include "mqttComm.h"

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

constexpr auto TAG_MAIN = "MAIN";

extern const int LED;
extern const int LED_ON;


bool ledOn = false;
bool ledChanged = false;

extern std::list <AsyncWebSocketClient*> clients;

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    if (!LittleFS.begin ()) {
        DEBUG_INFO (TAG_MAIN, "LittleFS Mount Failed");
        return;
    }
    if (readConfigFromFlash ()){
        DEBUG_INFO (TAG_MAIN, "Configuracion leida de flash");
    } else {
        DEBUG_INFO (TAG_MAIN, "Configuracion no leida de flash");
    }
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    M5.begin ();
    
    initDisplay ();
    initTimeSync ();
    initWebServer ();
    initMqttComm ();
}

void loop () {
    M5.update ();
    mqttUpdate ();
    if (M5.BtnA.wasReleased ()) {
        ledOn = !ledOn;
        ledChanged = true;
        sendButtonMqtt (ledOn);
    }
    if (ledChanged) {
        ledChanged = false;
        DEBUG_INFO (TAG_MAIN, "Led changed");
        if (saveConfigToFlash ()) {
            DEBUG_INFO (TAG_MAIN, "Configuracion guardada en flash");
        } else {
            DEBUG_INFO (TAG_MAIN, "Configuracion no guardada en flash");
        }

        for (AsyncWebSocketClient* client : clients) {
            client->printf ("{\"led\":%d}", ledOn ? 1 : 0);
        }
        digitalWrite (LED, ledOn ? LED_ON : !LED_ON);
    }
}