#include <Arduino.h>
#include "driver/ledc.h"

//constexpr int LEDC_CHANNEL_0 = 0;
constexpr int LEDC_BASE_FREQ = 5000;
constexpr int LEDC_RESOLUTION = LEDC_TIMER_8_BIT;

void ledcAnalogWrite (uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
    static uint32_t max_range = pow (2, LEDC_RESOLUTION) - 1;
    uint32_t duty = (max_range / valueMax) * min (value, valueMax);
    Serial.println (max_range - duty);

    // write duty to LEDC
    ledcWrite (channel, duty);
}

void setup () {
    Serial.begin (9600);
    //pinMode (BUILTIN_LED, OUTPUT);
    ledcSetup (LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin (BUILTIN_LED, LEDC_CHANNEL_0);
}

void loop () {
    static int brightness = 0;    // how bright the LED is
    static int fadeAmount = 5;    // how many points to fade the LED by
    int max_value = 255;
    //Serial.printf("¡Hola Mundo!\n");
    ledcAnalogWrite (LEDC_CHANNEL_0, brightness, max_value);
    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= max_value) {
        fadeAmount = -fadeAmount;
    }
    delay (30);
}
