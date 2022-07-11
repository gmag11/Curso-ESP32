#include "configStorage.h"
#include <Preferences.h>
#include <QuickDebug.h>

//const char* configFileName = "/config.json";

extern bool ledChanged;
extern bool ledOn;

constexpr auto TAG_STORAGE = "STORAGE";

Preferences preferences;
constexpr auto spaceName = "led";

void beginStorage () {
    preferences.begin (spaceName);
}


bool readConfigFromFlash () {
    ledOn = preferences.getBool ("ledOn", false);
    ledChanged = true;
    Serial.printf ("LEd Status loaded LED=%d\n", ledOn);
    return true;
}

bool saveConfigToFlash () {
    preferences.putBool ("ledOn", ledOn);
    Serial.printf ("Led status saved LED=%d\n", ledOn);
    return true;
}
