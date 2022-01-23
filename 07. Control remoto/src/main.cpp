#include <Arduino.h>
#include <WiFi.h>
#include <ESPNtpClient.h>
#include <ESPAsyncWebServer.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

constexpr auto LED = 5;
//constexpr auto LED = 19;
constexpr auto LED_ON = LOW;

AsyncWebServer server (80);

void notFound (AsyncWebServerRequest* request) {
    String response;
    response = request->url ();
    response += " not found";
    request->send (404, "text/plain", response);
}

void setup () {
    Serial.begin (9600);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    Serial.printf ("\nWiFi Connected. IP address: %s\n", WiFi.localIP ().toString ().c_str ());
    NTP.begin ("192.168.123.61", false);
    NTP.setTimeZone (TZ_Europe_Madrid);
    // NTP.onNTPSyncEvent ([] (NTPEvent_t ntpEvent) {
    //     log_printf ("NTP Event: %d: %s\n", ntpEvent, NTP.ntpEvent2str (ntpEvent));
    //                     });
    server.on ("/ledon", HTTP_GET, [] (AsyncWebServerRequest* request) {
        digitalWrite (LED, LED_ON);
        String response = NTP.getTimeDateStringForJS ();
        response += ": LED ON";
        request->send (200, "text/plain", response);
               });
    server.on ("/ledoff", HTTP_GET, [] (AsyncWebServerRequest* request) {
        digitalWrite (LED, !LED_ON);
        String response = NTP.getTimeDateStringForJS ();
        response += ": LED OFF";
        request->send (200, "text/plain", response);
               });
    server.on ("/led", HTTP_GET, [] (AsyncWebServerRequest* request) {
        String response = NTP.getTimeDateStringForJS ();
        response += ": LED is";
        response += (digitalRead (LED) == LED_ON) ? " ON" : " OFF";
        request->send (200, "text/plain", response);
               });
    server.onNotFound (notFound);
    server.begin ();

}

void loop () {}