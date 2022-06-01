#include <Arduino.h>
#include <WiFi.h>
#include <ESPNtpClient.h>

#ifdef ARDUINO_M5Stick_C
#include <M5StickCPlus.h>
#endif // ARDUINO_M5Stick_C

#if __has_include("wificonfig.h")
#include "wificonfig.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

#ifdef ARDUINO_M5Stick_C
constexpr auto LED = 10;
constexpr auto LED_ON = LOW;
#else
constexpr auto LED = 5;
constexpr auto LED_ON = LOW;
#endif // ARDUINO_M5Stick_C



TimerHandle_t ledTimer;
TaskHandle_t tareaMensaje = NULL;

constexpr auto periodoLED = 1000;
constexpr auto ledEncendido = 50; // ms
constexpr auto ledApagado = 150; // ms

constexpr auto esperaMensaje = 1000;

constexpr auto framerate = 4;
constexpr auto frametime = 1000 / framerate;

#ifdef ARDUINO_M5Stick_C
TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);
#endif // ARDUINO_M5Stick_C

/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

void parpadeaLED (void* pvParameters) {
    // for (;;) {
    //     time_t start = millis (); // toma la referencia del inicio

    //     //--------------------- Procesado visual -------------------------------------------
    //     time_t ciclo = millis () % periodoLED;

    //     if ((ciclo > 0 && ciclo < ledEncendido) || (ciclo > ledEncendido + ledApagado && ciclo < ledEncendido * 2 + ledApagado)) {
    //         digitalWrite (LED, LED_ON);
    //     } else {
    //         digitalWrite (LED, !LED_ON);
    //     }
    //     //----------------------------------------------------------------------------------
        
    //     delay (frametime - (millis () - start)); // Esperar el tiempo necesario para 40 fps
    // }

    log_printf ("Incio LED %u\n", millis());
    for (int i = 0; i < 2; i++) {
        digitalWrite (LED, LED_ON);
        delay (ledEncendido);
        digitalWrite (LED, !LED_ON);
        delay (ledApagado);
    }
    log_printf ("Fin LED %d\n", LED);
    // time_t cicloRestante = millis ();
    // cicloRestante = periodoLED - cicloRestante % periodoLED;
    // xTimerStart (ledTimer, cicloRestante / portTICK_PERIOD_MS);

}

#ifdef ARDUINO_M5Stick_C
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
#endif // ARDUINO_M5Stick_C

void escribeMensaje (void* pvParameters) {
    for (;;) {
        log_printf ("\nHola mundo. Ya estoy en Internet\n");
        log_printf ("Mi IP es %s\n", WiFi.localIP ().toString ().c_str ());
        log_printf ("Sabes qué hora es?... %s\n", NTP.getTimeDateString());
#ifdef ARDUINO_M5Stick_C
        updateDisplay ();
#endif // ARDUINO_M5Stick_C
        delay (esperaMensaje);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, LED_ON);
    delay (100);
    digitalWrite (LED, !LED_ON);
    ledTimer = xTimerCreate ("LED", periodoLED / portTICK_PERIOD_MS, pdTRUE, NULL, parpadeaLED);
    
    time_t cicloRestante = millis ();
    cicloRestante = periodoLED - (cicloRestante % periodoLED);
    vTaskDelay (cicloRestante / portTICK_PERIOD_MS);
    xTimerStart (ledTimer, pdMS_TO_TICKS (5000));
    log_printf ("Millis: %u, Ciclo restante %d\n", millis(), cicloRestante);

    WiFi.mode (WIFI_STA);
    //WiFi.enableLongRange (false);
    WiFi.begin (SSID, PASSWORD);

#ifdef ARDUINO_M5Stick_C
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
#endif // ARDUINO_M5Stick_C

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    WiFi.setAutoReconnect (false);
    NTP.begin ("es.pool.ntp.org", false);
    NTP.setTimeZone (TZ_Europe_Madrid);
    NTP.onNTPSyncEvent ([] (NTPEvent_t ntpEvent) {
        log_printf ("NTP Event: %d: %s\n", ntpEvent, NTP.ntpEvent2str(ntpEvent));
    });

    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
    //xTaskCreate (parpadeaLED, "LED", 2048, NULL, 1, &tareaLED);
}

void loop () {
    //time_t start = millis (); // toma la referencia del inicio

        //--------------------- Procesado visual -------------------------------------------
    // time_t ciclo = millis () % periodoLED;

    // if ((ciclo > 0 && ciclo < ledEncendido) || (ciclo > ledEncendido + ledApagado && ciclo < ledEncendido * 2 + ledApagado)) {
    //     digitalWrite (LED, LED_ON);
    // } else {
    //     digitalWrite (LED, !LED_ON);
    // }
    //----------------------------------------------------------------------------------
}