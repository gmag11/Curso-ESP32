#include <Arduino.h>
#include <WiFi.h>
//#include <LittleFS.h>
#include <QuickDebug.h>

#include <M5StickCPlus.h>
#include <list>
#include "webServer.h"
#include "configStorage.h"
#include "display.h"
#include <QuickEspNow.h>
#include <ArduinoJson.h>

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

void dataTx_cb (uint8_t* address, uint8_t status) {
    DEBUG_INFO (TAG_MAIN, "dataTx_cb(): status=%d", status);
}

void dataRx_cb (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
    StaticJsonDocument<32> doc;

    if (len < 2) {
        DEBUG_INFO (TAG_MAIN, "Message without data");
        return;
    }
    DeserializationError error = deserializeJson (doc, data, len);

    if (error) {
        DEBUG_ERROR (TAG_MAIN, "deserializeJson() failed: %s", error.c_str());
        return;
    }

    bool _ledOn = doc["led"];
    if (_ledOn != ledOn) {
        ledOn = _ledOn;
        ledChanged = true;
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    beginStorage ();
    if (readConfigFromFlash ()) {
        DEBUG_INFO (TAG_MAIN, "Configuracion leida de flash");
    } else {
        DEBUG_INFO (TAG_MAIN, "Configuracion no leida de flash");
    }
    WiFi.mode (WIFI_STA);
    WiFi.disconnect ();
    
    M5.begin ();
    
    initDisplay ();
    quickEspNow.onDataRcvd (dataRx_cb);
    quickEspNow.onDataSent (dataTx_cb);
    quickEspNow.begin ();
    
}

void loop () {
    M5.update ();
    if (M5.BtnA.wasReleased ()) {
        ledOn = !ledOn;
        ledChanged = true;
        StaticJsonDocument<16> doc;
        doc["led"] = ledOn;
        String jsonStr;
        serializeJson (doc, jsonStr);
        DEBUG_INFO (TAG_MAIN, "Data: " ARDUHAL_LOG_COLOR(ARDUHAL_LOG_COLOR_YELLOW) "%s", jsonStr.c_str());
        if (quickEspNow.send (ESPNOW_BROADCAST_ADDRESS, (const uint8_t*)jsonStr.c_str (), jsonStr.length ())) {
            DEBUG_ERROR (TAG_MAIN, "Mensaje no enviado");
        } else {
            DEBUG_INFO (TAG_MAIN, "Mensaje enviado");
        }
    }
    if (ledChanged) {
        ledChanged = false;
        DEBUG_INFO (TAG_MAIN, "Led changed");
        if (saveConfigToFlash ()) {
            DEBUG_INFO (TAG_MAIN, "Configuracion guardada en flash");
        } else {
            DEBUG_INFO (TAG_MAIN, "Configuracion no guardada en flash");
        }

        digitalWrite (LED, ledOn ? LED_ON : !LED_ON);
    }
}