#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <LittleFS.h>
#include <sntp.h>
#include <M5StickCPlus.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
constexpr auto NTP_SERVER = "time.cloudflare.com";
#endif

constexpr auto LED = 10;
//constexpr auto LED = 19;
constexpr auto LED_ON = LOW;

TimerHandle_t tareaDisplay = NULL;

bool timeSyncd = false;

bool ledOn = false;

AsyncWebServer server (80);
AsyncWebSocket ws ("/ws");
AsyncWebSocketClient* wsClient;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

void updateDisplay (void* pvParameters) {
    RTC_TimeTypeDef sTime;
    constexpr auto periodoDisplay = 10;

    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;
    static String ssid;
    static String localip;

    if (millis () - lastMeasurement > 1000) {
        lastMeasurement = millis ();
        current = M5.Axp.GetBatCurrent ();
        voltage = M5.Axp.GetBatVoltage ();
        ssid = WiFi.SSID ();
        localip = WiFi.localIP ().toString ();
    }

        //--------------------- Procesado visual -------------------------------------------
    if (millis () - lastDisplayUpdate >= periodoDisplay) {
        lastDisplayUpdate = millis ();
        if (ledOn) {
            Disbuff.fillSprite (BLUE);
        } else {
            Disbuff.fillSprite (BLACK);
        }
        Disbuff.setCursor (10, 10);
        Disbuff.setTextColor (WHITE);
        Disbuff.setTextSize (4);
        M5.Rtc.GetTime (&sTime);
        Disbuff.printf ("%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
        Disbuff.setCursor (10, 50);
        Disbuff.setTextSize (2);
        Disbuff.setTextColor (RED);
        Disbuff.printf ("%.2f V %.2f mA", voltage, current);
        Disbuff.setCursor (10, 75);
        Disbuff.setTextColor (WHITE);
        Disbuff.printf ("%s", ssid.c_str ());
        Disbuff.setCursor (10, 100);
        Disbuff.printf ("IP: %s", localip.c_str ());
        Disbuff.pushSprite (0, 0);
    }
    //----------------------------------------------------------------------------------
}

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
    if (LittleFS.exists (pathWithGz) || LittleFS.exists (path)) {
        if (LittleFS.exists (pathWithGz)) {
            path += ".gz";
        }
        log_d ("Content type: %s\r\n", contentType.c_str ());
        AsyncWebServerResponse* response = request->beginResponse (LittleFS, path, contentType);
        if (path.endsWith (".gz"))
            response->addHeader ("Content-Encoding", "gzip");
        //File file = LittleFS.open(path, "r");
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

void time_sync_cb (timeval* ntptime) {
    time_t hora_actual = time (NULL);

    tm* timeinfo = localtime (&hora_actual);
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;

    sTime.Hours = timeinfo->tm_hour;
    sTime.Minutes = timeinfo->tm_min;
    sTime.Seconds = timeinfo->tm_sec;

    M5.Rtc.SetTime (&sTime);

    sDate.WeekDay = timeinfo->tm_wday;
    sDate.Month = timeinfo->tm_mon + 1;
    sDate.Date = timeinfo->tm_mday;
    sDate.Year = timeinfo->tm_year + 1900;

    M5.Rtc.SetData (&sDate);

    timeSyncd = true;
}

void onWsEvent (AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        wsClient = client;
        Serial.println ("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        wsClient = nullptr;
        Serial.println ("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        Serial.printf ("Data: %.*s\n", len, (char*) data);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    if (!LittleFS.begin ()) {
        Serial.println ("LittleFS Mount Failed");
        return;
    }
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    M5.begin ();
    M5.Lcd.setRotation (3);
    Disbuff.createSprite (240, 135);
    Disbuff.setRotation (3);
    Disbuff.fillSprite (BLACK);
    Disbuff.setTextColor (WHITE);
    Disbuff.setTextSize (2);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("Conectando");
    Disbuff.pushSprite (0, 0);
    delay (100);

    sntp_setoperatingmode (SNTP_OPMODE_POLL);
    sntp_set_sync_mode (SNTP_SYNC_MODE_SMOOTH);
    sntp_setservername (0, NTP_SERVER);
    setenv ("TZ", PSTR ("CET-1CEST,M3.5.0,M10.5.0/3"), 1);
    tzset ();
    sntp_set_time_sync_notification_cb (time_sync_cb);
    sntp_init ();

    const uint fps = 10;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS (1000 / fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
    
    server.on ("/ledon", HTTP_GET, [] (AsyncWebServerRequest* request) {
        digitalWrite (LED, LED_ON);
        ledOn = true;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();        response += ": LED ON";
        request->send (200, "text/plain", response);
               });
    server.on ("/ledoff", HTTP_GET, [] (AsyncWebServerRequest* request) {
        digitalWrite (LED, !LED_ON);
        ledOn = false;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();        response += ": LED OFF";
        request->send (200, "text/plain", response);
               });
    server.on ("/led", HTTP_GET, [] (AsyncWebServerRequest* request) {
        String response = (digitalRead (LED)==LED_ON) ? "1" : "0";
        request->send (200, "text/plain", response);
               });

    server.onNotFound (notFound);

    server.addHandler (&ws);
    ws.onEvent (onWsEvent);

    server.begin ();

    

}

void loop () {
    M5.update ();
    if (M5.BtnA.wasReleased ()) {
        ledOn = !ledOn;
        digitalWrite (LED, ledOn ? LED_ON : !LED_ON);
        if (wsClient) {
            wsClient->printf("{\"led\":%d}", ledOn ? 1 : 0);
        }
        }
}