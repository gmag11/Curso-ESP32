#include <Arduino.h>
#include <WiFi.h>
#include <sntp.h>
#include <M5StickCPlus.h>
#include <HTTPClient.h>
#include <QuickDebug.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
constexpr auto NTP_SERVER = "time.cloudflare.com";
#endif

// constexpr auto LED = 5;
constexpr auto LED = 10;
constexpr auto LED_ON = LOW;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;
TimerHandle_t tareaDisplay = NULL;

bool timeSyncd = false;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);
float temp = -200;

float rate = 0;
bool ticker = false;
const String COIN = "ethereum";
const char* COINSHRT = "ETH";
const String FIAT = "usd";

const char* TAG = "WEBCLIENT";

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
        if (ticker) {
            if (temp > -100) {
                Disbuff.setTextColor (GREEN);
                Disbuff.printf ("T:%.2f C", temp);
            } else {
                Disbuff.setTextColor (RED);
                Disbuff.printf ("T:--.-- C");
            }
        } else {
            Disbuff.setTextColor (YELLOW);
            Disbuff.setTextSize (3);
            Disbuff.printf ("%s:%0.2f", COINSHRT, rate);
        }

        Disbuff.pushSprite (0, 0);
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

    const uint fps = 10;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS(1000/fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {
    static time_t lastWeatherUpdate = -290000;
    const time_t wuperiod = 1000 * 60 * 5;
    const String POBLACION = "Cubillos del Sil";

    if (millis () - lastWeatherUpdate > wuperiod) {
        lastWeatherUpdate = millis ();
        HTTPClient client;
        client.begin ("http://api.openweathermap.org/data/2.5/weather?units=metric&q=" + POBLACION + "&appid=" + OWM_API_KEY);
        int httpCode = client.GET ();
        if (httpCode == 200) {
            String payload = client.getString ();
            DEBUG_INFO (TAG, "%s", payload.c_str ());
            temp = payload.substring (payload.indexOf ("\"temp\":") + 7, payload.indexOf (",\"feels_like\"")).toFloat();
            DEBUG_INFO (TAG, "Temperatura: %f", temp);
        } else {
            DEBUG_ERROR (TAG, "Error: %d %s", httpCode, client.errorToString (httpCode).c_str ());
            temp = -200;
        }
    }

    static time_t lastBTCupdate = 0;
    const time_t btcPeriod = 1000 * 60 / 51;
    if (millis () - lastBTCupdate > btcPeriod) {
        lastBTCupdate = millis ();
        HTTPClient client;
        client.begin ("https://api.coingecko.com/api/v3/simple/price?ids=" + COIN + "&vs_currencies=" + FIAT);
        int httpCode = client.GET ();
        if (httpCode == 200) {
            String payload = client.getString ();
            DEBUG_INFO (TAG, "%s", payload.c_str ());
            payload = payload.substring (payload.indexOf ("\"" + FIAT + "\":"));
            DEBUG_INFO (TAG, "%s", payload.c_str ());
            rate = payload.substring (payload.indexOf (":") + 1, payload.indexOf ("}")).toFloat ();
            DEBUG_INFO (TAG, "%s: %f", COINSHRT, rate);
        } else {
            DEBUG_ERROR (TAG, "Error: %d %s", httpCode, client.errorToString (httpCode).c_str ());
            rate = 0;
        }
    }

    static time_t lastTickerChange = 0;
    const time_t tickerPeriod = 1000 * 5;

    if (millis() - lastTickerChange > tickerPeriod) {
        lastTickerChange = millis();
        ticker = !ticker;
    }

}