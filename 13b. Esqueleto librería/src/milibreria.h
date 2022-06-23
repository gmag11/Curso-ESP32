#ifndef MI_LIBRERIA_H
#define MI_LIBRERIA_H

#include <Arduino.h>

constexpr auto TAG_MILIB = "MILIB";

class MiLibreria {
public:
    MiLibreria();
    ~MiLibreria ();
    void saludar ();
    int getValor ();
    bool setValor (int valor);
    
protected:
    int valorOculto;
};



#endif // MI_LIBRERIA_H