#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>

const int LED = 10;
const int LED_ON = LOW;

bool readConfigFromFlash ();
bool saveConfigToFlash ();

#endif // CONFIG_H
