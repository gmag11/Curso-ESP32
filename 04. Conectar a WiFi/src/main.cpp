#include <Arduino.h>
#include <WiFi.h>

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

constexpr auto periodoLED = 1000;
constexpr auto ledEncendido = 50; // ms
constexpr auto ledApagado = 150; // ms

constexpr auto esperaMensaje = 10000;

constexpr auto framerate = 20;
constexpr auto frametime = 1000 / framerate;

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

void escribeMensaje (void* pvParameters) {
    for (;;) {
        log_printf ("\nHola mundo. Ya estoy en Internet\n");
        log_printf ("Mi IP es %s\n", WiFi.localIP ().toString ().c_str ());
        delay (esperaMensaje);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    WiFi.mode (WIFI_STA);
    WiFi.begin (SSID, PASSWORD);

    while (WiFi.status () != WL_CONNECTED) {
        delay (100);
        Serial.print (".");
    }
    
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}