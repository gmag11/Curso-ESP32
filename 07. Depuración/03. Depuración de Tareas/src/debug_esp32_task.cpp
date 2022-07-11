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
    LOG_IF_CODE (WARN, TAG, giveNumber (-1), -1, "Warning message if code");
    LOG_IF_CODE (ERROR, TAG, giveNumber (-2), -2, "Error message if code");
    Serial.println ();
    setTagDebugLevel (TAG, INFO);
    Serial.printf ("debug levels: %s:%d:%s %s:%d:%s\n", TAG, getTagDebugLevel (TAG), getTagDebugLevelStr (TAG).c_str (), TAG2, getTagDebugLevel (TAG2), getTagDebugLevelStr (TAG2).c_str ());
}

void task2 (void* param) {
    const char* tag = "TASK2";

    for (;;) {
        DEBUG_INFO (tag, "Task 2");
        vTaskDelay (pdMS_TO_TICKS (1100));
    }
}

TaskHandle_t task2Handle;

void setup () {
    Serial.begin (115200);
    Serial.println ();
    delay (2000);
    showDebug ();
    delay (1000);

    TaskHandle_t task2Handle;
    
    xTaskCreate (task2, "task2", 2048, NULL, 1, &task2Handle);
}

int* a;

void loop () {
    delay (2000);
    a = (int*)malloc (10 * sizeof (int)); // This will cause heap to be reduced
    DEBUG_INFO (TAG, "Pointer: %p", a); // You will see heap size reducing if memory is not freed [H: xxxxx]
    // free (a); // Memory is not freed if this line is commented out
}