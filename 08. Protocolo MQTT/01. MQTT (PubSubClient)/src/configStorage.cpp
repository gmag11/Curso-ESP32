#include "configStorage.h"
#include <QuickDebug.h>

const char* configFileName = "/config.json";

extern bool ledChanged;
extern bool ledOn;

constexpr auto TAG_STORAGE = "STORAGE";

bool readConfigFromFlash () {
    if (LittleFS.begin ()) {
        DEBUG_INFO (TAG_STORAGE, "LittleFS mounted");
        if (LittleFS.exists (configFileName)) {
            DEBUG_INFO (TAG_STORAGE, "Config file exists");
            File configFile = LittleFS.open (configFileName, "r");
            if (configFile) {
                DEBUG_INFO (TAG_STORAGE, "Config file opened");
                //String line;

                StaticJsonDocument<32> doc;
                DeserializationError error = deserializeJson (doc, configFile);

                if (error) {
                    DEBUG_INFO (TAG_STORAGE, "deserializeJson() failed: %s", error.c_str ());
                    return false;
                }

                int led = doc["led"];
                ledOn = led == 1;
                ledChanged = true;
                DEBUG_INFO (TAG_STORAGE, "LED=%d\n", ledOn);
                return true;
            } else {
                DEBUG_WARN (TAG_STORAGE, "Config file not opened");
                return false;
            }
        } else {
            DEBUG_INFO (TAG_STORAGE, "Config file does not exist");
            return false;
        }
    } else {
        DEBUG_WARN (TAG_STORAGE, "LittleFS not mounted");
        return false;
    }
    return false;
}

bool saveConfigToFlash () {
    if (LittleFS.begin ()) {
        DEBUG_INFO (TAG_STORAGE, "LittleFS mounted");
        File configFile = LittleFS.open (configFileName, "w", true);
        if (configFile) {
            DEBUG_INFO (TAG_STORAGE, "Config file opened");
            StaticJsonDocument<16> doc;
            doc["led"] = ledOn ? 1 : 0;
            serializeJson (doc, configFile);
            configFile.close ();
            return true;
        }
    }
    return false;
}
