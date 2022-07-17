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
        vTaskDelay (pdMS_TO_TICKS (esperaLED));
        digitalWrite (BUILTIN_LED, LOW);
        vTaskDelay (pdMS_TO_TICKS (esperaLED));
    }
}

void escribeMensaje (void* pvParameters) {
    for (;;) {
        Serial.printf ("Hola mundo: %d\n", counter);
        counter++;
        vTaskDelay (pdMS_TO_TICKS(esperaMensaje));
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (BUILTIN_LED, OUTPUT);
    xTaskCreate (parpadeaLED, "LED", 2048, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}