#include <Arduino.h>
#include <WiFi.h>
#include <sntp.h>
#include <M5StickCPlus.h>
#include <QuickDebug.h>
#include <OpenWeatherMapESP32.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
constexpr auto NTP_SERVER = "time.cloudflare.com";
const String OWM_API_KEY = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
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
float hum = -1;
float pressure = 0;

float rate = 0;

typedef enum ticker_state {
    TEMPERATURA,
    HUMEDAD,
    PRESION,
} ticker_state_t;

ticker_state_t ticker = TEMPERATURA;

const char* TAG = "SENSOR";

OpenWeatherMapESP32 owm;
const char* ciudad = "Madrid";

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
        switch (ticker) {
        case TEMPERATURA:
            Disbuff.setTextSize (4);
            if (temp > -100) {
                Disbuff.setTextColor (GREEN);
                Disbuff.printf ("T:%.2f C", temp);
            } else {
                Disbuff.setTextColor (RED);
                Disbuff.printf ("T:--.-- C");
            }
            break;
        case HUMEDAD:
            Disbuff.setTextSize (4);
            if (hum >= 0) {
                Disbuff.setTextColor (GREEN);
                Disbuff.printf ("H:%.2f %%", hum);
            } else {
                Disbuff.setTextColor (RED);
                Disbuff.printf ("H:--.-- %%");
            }
            break;
        default:
            Disbuff.setTextSize (3);
            if (pressure > 0) {
                Disbuff.setTextColor (GREEN);
                Disbuff.printf ("P:%.0f hPa", pressure);
            } else {
                Disbuff.setTextColor (RED);
                Disbuff.printf ("P:----- hPa");
            }
            break;
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
    WiFi.begin (SSID, PASSWORD);

    M5.begin ();
    M5.Axp.begin ();
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

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS (1000 / fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);

    owm.begin (OWM_API_KEY, ciudad);
}

void loop () {
    static time_t lastSensorUpdate = -290000;
    const time_t sensorPeriod = 250;

    if (millis () - lastSensorUpdate > sensorPeriod) {
        lastSensorUpdate = millis ();
        if (owm.askForWeather ()) {
            temp = owm.getTemperature ();
            hum = owm.getHumidity ();
            pressure = owm.getPressure ();
        } else {
            temp = -200;
            hum = -1;
            pressure = 0;
        }
        DEBUG_INFO (TAG, "T:%.2f C H:%.2f %% P:%.0f hPa", temp, hum, pressure);
    }

    static time_t lastTickerChange = 0;
    const time_t tickerPeriod = 1000 * 5;

    if (millis () - lastTickerChange > tickerPeriod) {
        lastTickerChange = millis ();
        ticker = (ticker_state_t)((ticker + 1) % 3);
    }

}