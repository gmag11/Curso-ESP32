#include <Arduino.h>
#include "driver/ledc.h"

constexpr int LEDC_BASE_FREQ = 5000;
constexpr int LEDC_RESOLUTION = LEDC_TIMER_8_BIT;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto esperaLED = 30;
constexpr auto esperaMensaje = 2000;
int counter = 0;

void ledcAnalogWrite (uint8_t canal, uint32_t valor, uint32_t valorMaximo = 255) {
  // calcular brillo, 4095 desde 2 ^ 12 - 1
    static uint32_t max_range = pow (2, LEDC_RESOLUTION) - 1;
    uint32_t brillo = (max_range / valorMaximo) * min (valor, valorMaximo);
    // Serial.println (max_range - brillo);

    // escribir brillo a LEDC
    ledcWrite (canal, brillo);
}

void loop_parpadeaLED (void* pvParameters) {
    for (;;) {
        static int brilloLED = 0;    // how bright the LED is
        static int cambioBrillo = 5;    // how many points to fade the LED by
        int maximo = 255;
        ledcAnalogWrite (LEDC_CHANNEL_0, brilloLED, maximo);
        brilloLED = brilloLED + cambioBrillo;
        if (brilloLED <= 0 || brilloLED >= maximo) {
            cambioBrillo = -cambioBrillo;
        }
        vTaskDelay (pdMS_TO_TICKS (esperaLED));
    }
}


// void loop_parpadeaLED (void* pvParameters) {
//     for (;;)   {
//         digitalWrite (BUILTIN_LED, HIGH);
//         vTaskDelay (pdMS_TO_TICKS (esperaLED));
//         digitalWrite (BUILTIN_LED, LOW);
//         vTaskDelay (pdMS_TO_TICKS (esperaLED));
//     }
// }

void loop_escribeMensaje (void* pvParameters) {
    for (;;) {
        Serial.printf ("Hola mundo: %d\n", counter);
        counter++;
        vTaskDelay (pdMS_TO_TICKS(esperaMensaje));
    }
}

void setup () {
    Serial.begin (115200);
    //pinMode (BUILTIN_LED, OUTPUT);
    ledcSetup (LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin (BUILTIN_LED, LEDC_CHANNEL_0);
    xTaskCreate (loop_parpadeaLED, "LED", 2048, NULL, 1, &tareaLED);
    xTaskCreate (loop_escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}