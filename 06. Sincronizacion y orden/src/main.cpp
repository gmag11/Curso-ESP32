#include <Arduino.h>
#include <WiFi.h>
#include <sntp.h>
#include <M5StickCPlus.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

// constexpr auto LED = 5;
constexpr auto LED = 10;
constexpr auto LED_ON = LOW;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;
TimerHandle_t tareaDisplay = NULL;

bool timeSyncd = false;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);


void updateDisplay (void* pvParameters) {
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
        time_t hora_actual = time (NULL);
        tm* hora = localtime (&hora_actual);
        Disbuff.printf ("%02d:%02d:%02d", hora->tm_hour, hora->tm_min, hora->tm_sec);
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

void parpadeaLED (void* pvParameters) {
    const int letFlashes = 2;
    const int ledONtime = 40;
    const int ledOFFtime = 100;
    const int ledCyclePeriod = 1000;


    for (;;) {
        time_t ciclo;
        static bool flashed = false;

        struct timeval tv_now;
        gettimeofday (&tv_now, NULL);
        time_t time_ms = tv_now.tv_sec * 1000L + tv_now.tv_usec / 1000L;

        if (timeSyncd) {
            ciclo = time_ms % ledCyclePeriod;;
        } else {
            ciclo = millis () % ledCyclePeriod;
        }
        
        if ((!flashed) && (ciclo >= 0 && ciclo < (ledONtime + ledOFFtime)*letFlashes)) {
            flashed = true;
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

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);

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

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    //WiFi.setAutoReconnect (false);

    sntp_setoperatingmode (SNTP_OPMODE_POLL);
    sntp_set_sync_mode (SNTP_SYNC_MODE_SMOOTH);
    sntp_setservername (0, "192.168.5.120");
    setenv ("TZ", PSTR ("CET-1CEST,M3.5.0,M10.5.0/3"), 1);
    tzset ();
    sntp_set_time_sync_notification_cb ([] (struct timeval* tv) {
        timeSyncd = true;
        log_printf ("\nHora sincronizada\n");
    });
    sntp_init ();

    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);

    xTaskCreate (parpadeaLED, "LED", 2048, NULL, 1, &tareaLED);

    const uint fps = 10;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS(1000/fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}