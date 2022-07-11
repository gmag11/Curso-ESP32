#include <QuickDebug.h>

static const char* TAG = "TAG";
static const char* TAG2 = "TAG2";

int32_t giveNumber (int32_t number) { // Mock function
    return number;
}

void showDebug () {
    setTagDebugLevel (TAG, WARN);
    DEBUG_ERROR (TAG, "Error message");
    DEBUG_WARN (TAG, "Warning message");
    DEBUG_INFO (TAG, "Info message");
    DEBUG_DBG (TAG, "Debug message");
    DEBUG_VERBOSE (TAG, ARDUHAL_LOG_COLOR (ARDUHAL_LOG_COLOR_MAGENTA) "Verbose message");
    Serial.println ();
    DEBUG_ERROR (TAG2, "Error message");
    DEBUG_WARN (TAG2, "Warning message");
    DEBUG_INFO (TAG2, "Info message");
    DEBUG_DBG (TAG2, "Debug message");
    DEBUG_VERBOSE (TAG2, ARDUHAL_LOG_COLOR (ARDUHAL_LOG_COLOR_MAGENTA) "Verbose message");
    Serial.println ();
    setTagToDefaultDebugLevel (TAG);
    DEBUG_ERROR (TAG, "Error message");
    DEBUG_WARN (TAG, "Warning message");
    DEBUG_INFO (TAG, "Info message");
    DEBUG_DBG (TAG, "Debug message");
    DEBUG_VERBOSE (TAG, ARDUHAL_LOG_COLOR (ARDUHAL_LOG_COLOR_MAGENTA) "Verbose message");
    Serial.println ();
    LOG_ERROR_IF_NON_ZERO (TAG, giveNumber (-1), "Error messageif non zero");
    LOG_ERROR_IF_ZERO (TAG, giveNumber (0), "Error message if zero");
    LOG_IF_CODE (WARN, TAG, giveNumber (-1), -1, "Warning message if code is -1");
    LOG_IF_CODE (ERROR, TAG, giveNumber (-2), -2, "Error message if code is -2");
    Serial.println ();
    setTagDebugLevel (TAG, INFO);
    Serial.printf ("debug levels: %s:%d:%s %s:%d:%s\n", TAG, getTagDebugLevel (TAG), getTagDebugLevelStr (TAG).c_str (), TAG2, getTagDebugLevel (TAG2), getTagDebugLevelStr (TAG2).c_str ());
}

void setup () {
    Serial.begin (115200);
    Serial.println ();
    delay (2000);
    showDebug ();
    delay (1000);
}

void loop () {
    delay (0);
}