#include <Arduino.h>

constexpr auto LED = 5;

void setup () {
    Serial.begin(9600);
    pinMode (LED, OUTPUT);
}

void loop () {
    Serial.println("¡Hola Mundo!");
    digitalWrite (LED, HIGH);
    delay (1000);
    digitalWrite (LED, LOW);
    delay (1000);
}