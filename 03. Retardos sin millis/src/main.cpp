#include <Arduino.h>

constexpr auto LED = 5;

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
            digitalWrite (LED, LOW);
        } else {
            digitalWrite (LED, HIGH);
        }
        //----------------------------------------------------------------------------------
        
        delay (frametime - (millis () - start)); // Esperar el tiempo necesario para 40 fps
    }
}

void escribeMensaje (void* pvParameters) {
    for (;;) {
        Serial.println ("Hola mundo");
        delay (esperaMensaje);
    }
}

void setup () {
    Serial.begin (9600);
    pinMode (LED, OUTPUT);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", configMINIMAL_STACK_SIZE, NULL, 1, &tareaMensaje);
}

void loop () {
}