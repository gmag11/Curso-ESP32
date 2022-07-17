#include <Arduino.h>
#include <WiFi.h>
#include <sntp.h>
#include <M5StickCPlus.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <QuickDebug.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
constexpr auto MQTT_SERVER = "time.gmprojects.pro";

#endif

// constexpr auto LED = 5;
constexpr auto LED = 10;
constexpr auto LED_ON = LOW;

TaskHandle_t tareaDisplay = NULL;
//TimerHandle_t tareaDisplay = NULL;

bool timeSyncd = false;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);
float temp = -200;

const char* TAG = "WEBCLIENT";

WiFiClientSecure secureClient;

// GlobalSign CA certificate valid until 28th January 2028
static const char SectigoCA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb
MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow
GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj
YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL
MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE
BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM
GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP
ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua
BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe
3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4
YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR
rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm
ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU
oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF
MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v
QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t
b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF
AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q
GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz
Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2
G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi
l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3
smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==
-----END CERTIFICATE-----
)EOF";

void updateDisplay (void* pvParameters) {
    RTC_TimeTypeDef sTime;
    constexpr auto periodoDisplay = 10;
    const uint fps = 10;

    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;
    static String ssid;
    static String localip;

    for (;;) {

        if (millis () - lastMeasurement >= 1000) {
            lastMeasurement = millis ();
            current = M5.Axp.GetBatCurrent ();
            voltage = M5.Axp.GetBatVoltage ();
            ssid = WiFi.SSID ();
            localip = WiFi.localIP ().toString ();
            DEBUG_INFO (TAG, "Display updated");
        }

            //--------------------- Procesado visual -------------------------------------------
        if (millis () - lastDisplayUpdate >= periodoDisplay) {
            lastDisplayUpdate = millis ();
            Disbuff.fillSprite (BLACK);
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
            Disbuff.setTextSize (4);
            if (temp > -100) {
                Disbuff.setTextColor (GREEN);
                Disbuff.printf ("T:%.2f C", temp);
            } else {
                Disbuff.setTextColor (RED);
                Disbuff.printf ("T:--.-- C");
            }
            Disbuff.pushSprite (0, 0);
        }

        vTaskDelay (pdMS_TO_TICKS (1000 / fps / 10));
    }
    //----------------------------------------------------------------------------------
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

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);

    WiFi.mode (WIFI_STA);
    //WiFi.enableLongRange (false);
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

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    //WiFi.setAutoReconnect (false);


    sntp_setoperatingmode (SNTP_OPMODE_POLL);
    sntp_set_sync_mode (SNTP_SYNC_MODE_SMOOTH);
    sntp_setservername (0, NTP_SERVER);
    setenv ("TZ", PSTR ("CET-1CEST,M3.5.0,M10.5.0/3"), 1);
    tzset ();
    sntp_set_time_sync_notification_cb (time_sync_cb);
    sntp_init ();

    //const uint fps = 10;

    //tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS (1000 / fps), pdTRUE, NULL, updateDisplay);
    //xTimerStart (tareaDisplay, 0);
    xTaskCreate (updateDisplay, "Display", 2048, NULL, 1, &tareaDisplay);
    configASSERT (tareaDisplay);
    secureClient.setCACert (SectigoCA);
}

void loop () {
    static time_t lastWeatherUpdate = -290000;
    const time_t wuperiod = 1000 * 60 * 5;
    const String POBLACION = "Cubillos del Sil";

    if (millis () - lastWeatherUpdate > wuperiod) {
        lastWeatherUpdate = millis ();
        HTTPClient client;
        client.begin (secureClient, "https://api.openweathermap.org/data/2.5/weather?units=metric&q=" + POBLACION + "&appid=" + OWM_API_KEY);
        int httpCode = client.GET ();
        if (httpCode == 200) {
            String payload = client.getString ();
            DEBUG_INFO (TAG, "%s", payload.c_str ());
            temp = payload.substring (payload.indexOf ("\"temp\":") + 7, payload.indexOf (",\"feels_like\"")).toFloat ();
            DEBUG_INFO (TAG, "Temperatura: %f", temp);
        } else {
            DEBUG_ERROR (TAG, "Error: %d %s", httpCode, client.errorToString (httpCode).c_str ());
            temp = -200;
        }
        client.end ();
        DEBUG_INFO (TAG, "Cliente cerrado");
    }
}