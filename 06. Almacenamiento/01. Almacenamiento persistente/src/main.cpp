#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
//#include <LittleFS.h>
#include <FFat.h>
#include <sntp.h>
//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>
#include <I2C_BM8563.h>
#include <JC_Button_ESP.h>
#include <list>

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
constexpr auto BUTTON_PIN = 37;

TimerHandle_t tareaDisplay = NULL;

bool timeSyncd = false;

bool ledOn = false;
bool ledChanged = false;

AsyncWebServer server (80);
AsyncWebSocket ws ("/ws");

std::list <AsyncWebSocketClient*> clients;

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
I2C_BM8563 rtc = I2C_BM8563 (I2C_BM8563_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);

Button BtnA (BUTTON_PIN);

const char* configFileName = "/config.conf";

bool readConfigFromFlash () {
    if (FFat.begin ()) {
        Serial.println ("FFat mounted");
        if (FFat.exists (configFileName)) {
            Serial.println ("Config file exists");
            File configFile = FFat.open (configFileName, "r");
            if (configFile) {
                Serial.println ("Config file opened");
                String line;
                while (configFile.available ()) {
                    line = configFile.readStringUntil ('\n');
                    int count = 1;
                    Serial.printf ("Linea %d: %s\n", count++, line);
                    if (line.startsWith ("LED=")) {
                        String led = line.substring (4);
                        led.trim ();
                        Serial.printf ("LED substring: *%s*\n", led.c_str ());
                        ledOn = led == "1";
                        ledChanged = true;
                        Serial.printf ("LED=%d\n", ledOn);
                        return true;
                    } else {
                        Serial.println ("Invalid line");
                        return false;
                    }
                }
            } else {
                Serial.println ("Config file not opened");
                return false;
            }
        } else {
            Serial.println ("Config file not exists");
            return false;
        }
    } else {
        Serial.println ("LittleFS not mounted");
        return false;
    }
    return false;
}

bool saveConfigToFlash () {
    if (FFat.begin ()) {
        Serial.println ("LittleFS mounted");
        File configFile = FFat.open (configFileName, "w", true);
        if (configFile) {
            Serial.println ("Config file opened");
            configFile.print ("LED=");
            configFile.println (ledOn ? "1" : "0");
            configFile.close ();
            return true;
        }
    }
    return false;
}

void updateDisplay (void* pvParameters) {
    I2C_BM8563_TimeTypeDef sTime;
    constexpr auto periodoDisplay = 10;

    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;
    static String ssid;
    static String localip;

    if (millis () - lastMeasurement > 1000) {
        lastMeasurement = millis ();
        current = axp192.getBatteryDischargeCurrent ();
        if (current <= 0.01) {
            current = -axp192.getBatteryChargeCurrent ();
        }
        voltage = axp192.getBatteryVoltage ();
        ssid = WiFi.SSID ();
        localip = WiFi.localIP ().toString ();
    }

        //--------------------- Procesado visual -------------------------------------------
    if (millis () - lastDisplayUpdate >= periodoDisplay) {
        lastDisplayUpdate = millis ();
        if (ledOn) {
            Disbuff.fillSprite (TFT_BLUE);
        } else {
            Disbuff.fillSprite (TFT_BLACK);
        }
        Disbuff.setCursor (10, 10);
        Disbuff.setTextSize (4);
        tm timeInfo;
        if (timeSyncd) {
            getLocalTime (&timeInfo);
            Disbuff.setTextColor (TFT_GREEN);
            Disbuff.printf ("%02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
        } else {
            rtc.getTime (&sTime);
            Disbuff.setTextColor (TFT_WHITE);
            Disbuff.printf ("%02d:%02d:%02d", sTime.hours, sTime.minutes, sTime.seconds);
        }
        Disbuff.setCursor (10, 50);
        Disbuff.setTextSize (2);
        Disbuff.setTextColor (TFT_RED);
        Disbuff.printf ("%.2f V %.2f mA", voltage, current);
        Disbuff.setCursor (10, 75);
        Disbuff.setTextColor (TFT_WHITE);
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
    if (FFat.exists (path) || FFat.exists (pathWithGz)) {
        if (FFat.exists (pathWithGz)) {
            path += ".gz";
        }
        log_d ("Content type: %s\r\n", contentType.c_str ());
        AsyncWebServerResponse* response = request->beginResponse (FFat, path, contentType);
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
    I2C_BM8563_DateTypeDef sDate;
    I2C_BM8563_TimeTypeDef sTime;

    sTime.hours = timeinfo->tm_hour;
    sTime.minutes = timeinfo->tm_min;
    sTime.seconds = timeinfo->tm_sec;

    rtc.setTime (&sTime);

    sDate.weekDay = timeinfo->tm_wday;
    sDate.month = timeinfo->tm_mon + 1;
    sDate.date = timeinfo->tm_mday;
    sDate.year = timeinfo->tm_year + 1900;

    rtc.setDate (&sDate);

    timeSyncd = true;
}

void onWsEvent (AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        clients.push_front (client);
        Serial.printf ("WebSocket client connected. Clients: %d\n", clients.size ());
    } else if (type == WS_EVT_DISCONNECT) {
        clients.remove (client);
        Serial.printf ("WebSocket client disconnected. Clients: %d\n", clients.size ());
    } else if (type == WS_EVT_DATA) {
        Serial.printf ("Data: %.*s\n", len, (char*) data);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    rtc.begin ();
    BtnA.begin ();

    if (!FFat.begin ()) {
        Serial.println ("LittleFS Mount Failed");
        return;
    }
    if (readConfigFromFlash ()){
        Serial.println ("Configuracion leida de flash");
    } else {
        Serial.println ("Configuracion no leida de flash");
    }
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    Wire1.begin (21, 22);
    Wire1.setClock (400000);

    I2C_AXP192_InitDef initDef = {
        .EXTEN = true,
        .BACKUP = true,
        .DCDC1 = 3300,
        .DCDC2 = 0,
        .DCDC3 = 0,
        .LDO2 = 2800,
        .LDO3 = 3000,
        .GPIO0 = 2800,
        .GPIO1 = -1,
        .GPIO2 = -1,
        .GPIO3 = -1,
        .GPIO4 = -1,
    };
    axp192.begin (initDef);
    lcd.init ();
    lcd.setRotation (1);
    lcd.fillScreen (TFT_BLACK);
    Disbuff.createSprite (TFT_HEIGHT, TFT_WIDTH);
    Disbuff.setRotation (1);
    Disbuff.fillSprite (TFT_BLACK);
    Disbuff.setTextColor (TFT_WHITE);
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
        //digitalWrite (LED, LED_ON);
        Serial.println("Led on");
        ledOn = true;
        ledChanged = true;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();
        response += ": LED ON";
        request->send (200, "text/plain", response);
        if (saveConfigToFlash ()) {
            Serial.println ("Configuracion guardada en flash");
        } else {
            Serial.println ("Configuracion no guardada en flash");
        }
               });
    server.on ("/ledoff", HTTP_GET, [] (AsyncWebServerRequest* request) {
        //digitalWrite (LED, !LED_ON);
        Serial.println ("Led off");
        ledOn = false;
        ledChanged = true;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();
        response += ": LED OFF";
        request->send (200, "text/plain", response);
        if (saveConfigToFlash ()) {
            Serial.println ("Configuracion guardada en flash");
        } else {
            Serial.println ("Configuracion no guardada en flash");
        }
               });
    server.on ("/led", HTTP_GET, [] (AsyncWebServerRequest* request) {
        String response = (digitalRead (LED)==LED_ON) ? "1" : "0";
        request->send (200, "text/plain", response);
               });
    server.on (configFileName, [] (AsyncWebServerRequest* request) {
        request->send (403, "text/plain", "Forbidden");
               });

    server.onNotFound (notFound);

    server.addHandler (&ws);
    ws.onEvent (onWsEvent);

    server.begin ();

    

}

void loop () {
    BtnA.read ();
    if (BtnA.wasReleased ()) {
        ledOn = !ledOn;
        ledChanged = true;
        if (saveConfigToFlash ()) {
            Serial.println ("Configuracion guardada en flash");
        } else {
            Serial.println ("Configuracion no guardada en flash");
        }
    }
    if (ledChanged) {
        ledChanged = false;
        Serial.println ("Led changed");
        for (AsyncWebSocketClient* client : clients) {
            client->printf ("{\"led\":%d}", ledOn ? 1 : 0);
        }
        digitalWrite (LED, ledOn ? LED_ON : !LED_ON);
    }
}