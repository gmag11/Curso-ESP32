#include <Arduino.h>

constexpr auto LED = 5;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto esperaLED = 300;
constexpr auto esperaMensaje = 2000;

void parpadeaLED (void* pvParameters) {
    for (;;)   {
        digitalWrite (LED, HIGH);
        delay (esperaLED);
        digitalWrite (LED, LOW);
        delay (esperaLED);
    }
}

void escribeMensaje (void* pvParameters) {
    for (;;)   {
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