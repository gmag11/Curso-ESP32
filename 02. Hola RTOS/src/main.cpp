#include <Arduino.h>

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto esperaLED = 300;
constexpr auto esperaMensaje = 2000;
int counter = 0;
bool sendMessage = false;

void parpadeaLED (void* pvParameters) {
    for (;;)   {
        digitalWrite (BUILTIN_LED, HIGH);
        delay (esperaLED);
        digitalWrite (BUILTIN_LED, LOW);
        delay (esperaLED);
    }
}

void escribeMensaje (void* pvParameters) {
    for (;;) {
        sendMessage = true;
        delay (esperaMensaje);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (BUILTIN_LED, OUTPUT);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", configMINIMAL_STACK_SIZE, NULL, 1, &tareaMensaje);
}

void loop () {
    if (sendMessage) {
        sendMessage = false;
        Serial.printf ("Hola mundo: %d\n", counter);
        counter++;
    }
}