#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <LittleFS.h>
#include <list>


void notFound (AsyncWebServerRequest* request);

void onWsEvent (AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

void initWebServer ();

#endif // WEB_SERVER_H