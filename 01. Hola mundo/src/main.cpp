#include <Arduino.h>

void setup () {
    Serial.begin(9600);
    pinMode (BUILTIN_LED, OUTPUT);
}

void loop () {
    Serial.printf("¡Hola Mundo!\n");
    digitalWrite (BUILTIN_LED, HIGH);
    delay (1000);
    digitalWrite (BUILTIN_LED, LOW);
    delay (1000);
}