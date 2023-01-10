#include <Arduino.h>
#include <WiFi.h>
#include <sntp.h>

//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>
#include <I2C_BM8563.h>

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

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
I2C_BM8563 rtc = I2C_BM8563 (I2C_BM8563_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);


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
        Disbuff.fillSprite (TFT_BLACK);
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

void parpadeaLED (void* pvParameters) {
    const int letFlashes = 2;
    const int ledONtime = 40;
    const int ledOFFtime = 100;
    const int ledCyclePeriod = 5000;


    for (;;) {
        time_t ciclo;
        static bool flashed = false;

        struct timeval tv_now;
        gettimeofday (&tv_now, NULL);
        uint64_t seconds_ms = (uint64_t)(tv_now.tv_sec) * 1000L;
        uint32_t useconds_ms = tv_now.tv_usec / 1000L;
        uint64_t time_ms = seconds_ms + (uint64_t)useconds_ms;
        if (timeSyncd) {
            ciclo = (uint64_t)time_ms % (uint64_t)ledCyclePeriod;
        } else {
            ciclo = millis () % ledCyclePeriod;
        }
        //log_printf("ciclo: %d ms\n", ciclo);

        if ((!flashed) && (ciclo >= 0 && ciclo < (ledONtime + ledOFFtime)*letFlashes)) {
            flashed = true;
            // log_printf ("Flashing ciclo: %d sec: %lu\n", ciclo, tv_now.tv_sec);
            for (int i = 0; i < letFlashes; i++) {
                digitalWrite (LED, LED_ON);
                delay (ledONtime);
                digitalWrite (LED, !LED_ON);
                delay (ledOFFtime);
            }
        }

        if (flashed && (ciclo > (ledONtime + ledOFFtime) * letFlashes)) {
            flashed = false;
        }
        delay (1);
    }
}

void escribeMensaje (void* pvParameters) {
    constexpr auto esperaMensaje = 10000;
    
    for (;;) {
        log_printf ("\nHola mundo. Ya estoy en Internet\n");
        log_printf ("Mi IP es %s\n", WiFi.localIP ().toString ().c_str ());
        time_t now = time (NULL);
        log_printf ("Sabes qué hora es?... %s\n", ctime(&now));

        delay (esperaMensaje);
    }
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

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    rtc.begin ();

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

    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);

    xTaskCreate (parpadeaLED, "LED", 2048, NULL, 1, &tareaLED);

    const uint fps = 10;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS(1000/fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}