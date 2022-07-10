#include <Arduino.h>
#include <QuickDebug.h>
#include <MiLibreria.h>

MiLibreria manejaValor;

void setup () {
    Serial.begin (115200);

    manejaValor.saludar ();

    if (manejaValor.setValor (0)) {
        DEBUG_INFO (TAG_MILIB, "Valor cambiado a 0");
    } else {
        DEBUG_INFO (TAG_MILIB, "Valor no cambiado");
    }
    delay (1000);

    if (manejaValor.setValor (11)) {
        DEBUG_INFO (TAG_MILIB, "Valor cambiado a 11");
    } else {
        DEBUG_INFO (TAG_MILIB, "Valor no cambiado");
    }
}

void loop () {
    DEBUG_INFO (TAG_MILIB, "Valor: %d", manejaValor.getValor ());
    delay (1000);
}