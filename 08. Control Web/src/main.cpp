#include <Arduino.h>
#include <WiFi.h>
#include <ESPNtpClient.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

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

String getContentType (String filename, AsyncWebServerRequest* request) {
    if (request->hasArg ("download")) return "application/octet-stream";
    else if (filename.endsWith (".htm")) return "text/html";
    else if (filename.endsWith (".html")) return "text/html";
    else if (filename.endsWith (".css")) return "text/css";
    else if (filename.endsWith (".js")) return "application/javascript";
    else if (filename.endsWith (".json")) return "application/json";
    else if (filename.endsWith (".png")) return "image/png";
    else if (filename.endsWith (".gif")) return "image/gif";
    else if (filename.endsWith (".jpg")) return "image/jpeg";
    else if (filename.endsWith (".ico")) return "image/x-icon";
    else if (filename.endsWith (".xml")) return "text/xml";
    else if (filename.endsWith (".pdf")) return "application/x-pdf";
    else if (filename.endsWith (".zip")) return "application/x-zip";
    else if (filename.endsWith (".gz")) return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead (String path, AsyncWebServerRequest* request) {
    log_d ("handleFileRead: %s\r\n", path.c_str ());
    if (path.endsWith ("/"))
        path += "index.htm";
    String contentType = getContentType (path, request);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists (pathWithGz) || SPIFFS.exists (path)) {
        if (SPIFFS.exists (pathWithGz)) {
            path += ".gz";
        }
        log_d ("Content type: %s\r\n", contentType.c_str ());
        AsyncWebServerResponse* response = request->beginResponse (SPIFFS, path, contentType);
        if (path.endsWith (".gz"))
            response->addHeader ("Content-Encoding", "gzip");
        //File file = SPIFFS.open(path, "r");
        log_d ("File %s exist\r\n", path.c_str ());
        request->send (response);
        log_d ("File %s Sent\r\n", path.c_str ());
        return true;
    }
    
    log_d ("Cannot find %s\n", path.c_str ());
    return false;
}



void notFound (AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse (200);
    response->addHeader ("Connection", "close");
    response->addHeader ("Access-Control-Allow-Origin", "*");
    if (!handleFileRead (request->url (), request)) {
        request->send (404, "text/plain", "FileNotFound");
        log_e ("Not found: %s\r\n", request->url ().c_str ());
    }
    delete response; // Free up memory!
}

void setup () {
    Serial.begin (9600);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    if (!SPIFFS.begin ()) {
        Serial.println ("SPIFFS Mount Failed");
        return;
    }
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    Serial.printf ("\nWiFi Connected. IP address: %s\n", WiFi.localIP ().toString ().c_str ());
    NTP.begin ("192.168.5.120", false);
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
        String response = (digitalRead (LED)==LED_ON) ? "1" : "0";
        request->send (200, "text/plain", response);
               });

    server.onNotFound (notFound);
    server.begin ();

}

void loop () {}