#include "webServer.h"
#include "configStorage.h"
#include <QuickDebug.h>

extern bool ledChanged;
extern bool ledOn;

extern const int LED;
extern const int LED_ON;

extern const char* configFileName;

AsyncWebServer server (80);
AsyncWebSocket ws ("/ws");

std::list <AsyncWebSocketClient*> clients;

constexpr auto TAG_STORAGE = "STORAGE";

String getContentType (String filename, AsyncWebServerRequest* request) {
    if (request->hasArg ("download")) return "application/octet-stream";
    else if (filename.endsWith (".htm")) return "text/html";
    else if (filename.endsWith (".html")) return "text/html";
    else if (filename.endsWith (".css")) return "text/css";
    else if (filename.endsWith (".js")) return "application/javascript";
    else if (filename.endsWith (".json")) return "application/json";
    else if (filename.endsWith (".png")) return "image/png";
    else if (filename.endsWith (".gif")) return "image/gif";
    else if (filename.endsWith (".jpg")) return "image/jpeg";
    else if (filename.endsWith (".ico")) return "image/x-icon";
    else if (filename.endsWith (".xml")) return "text/xml";
    else if (filename.endsWith (".pdf")) return "application/x-pdf";
    else if (filename.endsWith (".zip")) return "application/x-zip";
    else if (filename.endsWith (".gz")) return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead (String path, AsyncWebServerRequest* request) {
    log_d ("handleFileRead: %s\r\n", path.c_str ());
    if (path.endsWith ("/"))
        path += "index.htm";
    String contentType = getContentType (path, request);
    String pathWithGz = path + ".gz";
    if (LittleFS.exists (pathWithGz) || LittleFS.exists (path)) {
        if (LittleFS.exists (pathWithGz)) {
            path += ".gz";
        }
        log_d ("Content type: %s\r\n", contentType.c_str ());
        AsyncWebServerResponse* response = request->beginResponse (LittleFS, path, contentType);
        if (path.endsWith (".gz"))
            response->addHeader ("Content-Encoding", "gzip");
        //File file = LittleFS.open(path, "r");
        log_d ("File %s exist\r\n", path.c_str ());
        request->send (response);
        log_d ("File %s Sent\r\n", path.c_str ());
        return true;
    }

    log_d ("Cannot find %s\n", path.c_str ());
    return false;
}

void notFound (AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse (200);
    response->addHeader ("Connection", "close");
    response->addHeader ("Access-Control-Allow-Origin", "*");
    if (!handleFileRead (request->url (), request)) {
        request->send (404, "text/plain", "FileNotFound");
        log_e ("Not found: %s\r\n", request->url ().c_str ());
    }
    delete response; // Free up memory!
}

void onWsEvent (AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        clients.push_front (client);
        DEBUG_INFO (TAG_STORAGE, "WebSocket client connected. Clients: %d\n", clients.size ());
    } else if (type == WS_EVT_DISCONNECT) {
        clients.remove (client);
        DEBUG_INFO (TAG_STORAGE, "WebSocket client disconnected. Clients: %d\n", clients.size ());
    } else if (type == WS_EVT_DATA) {
        DEBUG_INFO (TAG_STORAGE, "Data: %.*s\n", len, (char*)data);
    }
}

void initWebServer () {
    server.on ("/ledon", HTTP_GET, [] (AsyncWebServerRequest* request) {
    //digitalWrite (LED, LED_ON);
        DEBUG_INFO (TAG_STORAGE, "Led on");
        ledOn = true;
        ledChanged = true;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();
        response += ": LED ON";
        request->send (200, "text/plain", response);
        if (saveConfigToFlash ()) {
            DEBUG_INFO (TAG_STORAGE, "Configuracion guardada en flash");
        } else {
            DEBUG_INFO (TAG_STORAGE, "Configuracion no guardada en flash");
        }
               });
    server.on ("/ledoff", HTTP_GET, [] (AsyncWebServerRequest* request) {
        //digitalWrite (LED, !LED_ON);
        DEBUG_INFO (TAG_STORAGE, "Led off");
        ledOn = false;
        ledChanged = true;
        time_t now = time (NULL);
        String response = ctime (&now);
        response.trim ();
        response += ": LED OFF";
        request->send (200, "text/plain", response);
        if (saveConfigToFlash ()) {
            DEBUG_INFO (TAG_STORAGE, "Configuracion guardada en flash");
        } else {
            DEBUG_INFO (TAG_STORAGE, "Configuracion no guardada en flash");
        }
               });
    server.on ("/led", HTTP_GET, [] (AsyncWebServerRequest* request) {
        String response = (digitalRead (LED) == LED_ON) ? "1" : "0";
        request->send (200, "text/plain", response);
               });
    server.on (configFileName, [] (AsyncWebServerRequest* request) {
        request->send (403, "text/plain", "Forbidden");
               });

    server.onNotFound (notFound);

    server.addHandler (&ws);
    ws.onEvent (onWsEvent);

    server.begin ();

}
