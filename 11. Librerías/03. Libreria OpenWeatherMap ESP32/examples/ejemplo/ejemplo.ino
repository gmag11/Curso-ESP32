#include <Arduino.h>
#include <QuickDebug.h>
#include <OpenWeatherMapESP32.h>

constexpr auto CIUDAD = "Madrid";

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

OpenWeatherMapESP32 owm;

void setup () {
    Serial.begin (115200);
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);
    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    Serial.println (" Conectado");
    owm.begin (OWM_API_KEY, CIUDAD);
}

void loop () {
    static time_t lastWeatherUpdate = 0;
    const time_t wuperiod = 1000;
    
    if (millis () - lastWeatherUpdate > wuperiod) {
        lastWeatherUpdate = millis ();
        Serial.println ("Pidiendo datos...");
        if(!owm.askForWeather ()) {
            Serial.println ("Error al pedir datos");
        } else {
            Serial.println ("Datos OK");
        }
        if (owm.isValid ()) {
            Serial.printf ("Temperature: %.2f ºC\n", owm.getTemperature ());
            Serial.printf ("Humidity: %.0f %%\n", owm.getHumidity ());
            Serial.printf ("Pressure: %.0f hPa\n", owm.getPressure ());
        }
    }  
}