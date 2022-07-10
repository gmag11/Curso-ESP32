#include "milibreria.h"

const int MAX = 19;
const int MIN = 0;

MiLibreria::MiLibreria () {
    valorOculto = 10;
}

MiLibreria::~MiLibreria () {}

void MiLibreria::saludar () {
    Serial.println ("Hola mundo");
}

int MiLibreria::getValor () {
    valorOculto++;
    if (valorOculto > MAX) {
        valorOculto = MIN;
    }
    return valorOculto;
}

bool MiLibreria::setValor (int valor) {
    if (valor >= MAX || valor < MIN) {
        return false;
    }
    valorOculto = valor;
    return true;
}