#ifndef MQTTCOMM_H
#define MQTTCOMM_H

#include <Arduino.h>

void initMqttComm ();
//void mqttUpdate ();
void sendButtonMqtt (bool state);

#endif // MQTTCOMM_H