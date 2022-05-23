#include <Arduino.h>
#include <WiFi.h>
#include <ESPNtpClient.h>
#include <M5StickCPlus.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

// constexpr auto LED = 5;
constexpr auto LED = 10;
// constexpr auto LED_ON = LOW;
constexpr auto LED_ON = LOW;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto periodoLED = 1000;
constexpr auto ledEncendido = 50; // ms
constexpr auto ledApagado = 150; // ms

constexpr auto esperaMensaje = 1000;

constexpr auto framerate = 20;
constexpr auto frametime = 1000 / framerate;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

void parpadeaLED (void* pvParameters) {
    for (;;) {
        time_t start = millis (); // toma la referencia del inicio

        //--------------------- Procesado visual -------------------------------------------
        time_t ciclo = millis () % periodoLED;

        if ((ciclo > 0 && ciclo < ledEncendido) || (ciclo > ledEncendido + ledApagado && ciclo < ledEncendido * 2 + ledApagado)) {
            digitalWrite (LED, LED_ON);
        } else {
            digitalWrite (LED, !LED_ON);
        }
        //----------------------------------------------------------------------------------
        
        delay (frametime - (millis () - start)); // Esperar el tiempo necesario para 40 fps
    }
}

void updateDisplay () {
    Disbuff.fillSprite (BLUE);
    Disbuff.setTextColor (WHITE);
    Disbuff.setTextSize (4);
    time_t hora_actual = time (NULL);
    tm* hora = localtime (&hora_actual);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("%02d:%02d:%02d", hora->tm_hour, hora->tm_min, hora->tm_sec);
    Disbuff.setCursor (10, 55);
    Disbuff.setTextSize (3);
    Disbuff.printf ("%.2f V", M5.Axp.GetBatVoltage ());
    Disbuff.setCursor (10, 86);
    Disbuff.setTextSize (2);
    Disbuff.printf ("IP: %s", WiFi.localIP ().toString ().c_str ());
    Disbuff.setCursor (10, 107);
    Disbuff.printf ("%s", WiFi.SSID ().c_str ());
    Disbuff.pushSprite (0, 0);
}

void escribeMensaje (void* pvParameters) {
    for (;;) {
        log_printf ("\nHola mundo. Ya estoy en Internet\n");
        log_printf ("Mi IP es %s\n", WiFi.localIP ().toString ().c_str ());
        log_printf ("Sabes qué hora es?... %s\n", NTP.getTimeDateString());
        updateDisplay ();
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
    Disbuff.fillSprite (BLUE);
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
    
    NTP.begin ("es.pool.ntp.org", false);
    NTP.setTimeZone (TZ_Europe_Madrid);
    NTP.onNTPSyncEvent ([] (NTPEvent_t ntpEvent) {
        log_printf ("NTP Event: %d: %s\n", ntpEvent, NTP.ntpEvent2str(ntpEvent));
    });

    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}