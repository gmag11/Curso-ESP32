#include <Arduino.h>

constexpr auto LED = 10;

void setup () {
    Serial.begin(9600);
    pinMode (LED, OUTPUT);
}

void loop () {
    Serial.printf("¡Hola Mundo!\n");
    digitalWrite (LED, HIGH);
    delay (1000);
    digitalWrite (LED, LOW);
    delay (1000);
}