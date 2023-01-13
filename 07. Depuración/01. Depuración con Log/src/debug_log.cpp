#include <Arduino.h>
#include <esp_log.h>

static const char* TAG = "TAG";
static const char* TAG2 = "TAG2";

int32_t giveNumber (int32_t number) { // Mock function
    return number;
}

void showDebug () {
    esp_log_level_set (TAG, ESP_LOG_WARN);
    ESP_LOGE (TAG, "Error message");
    ESP_LOGW (TAG, "Warning message");
    ESP_LOGI (TAG, "Info message");
    ESP_LOGD (TAG, "Debug message");
    ESP_LOGV (TAG, "Verbose message");
    Serial.println ();
    ESP_LOGE (TAG2, "Error message");
    ESP_LOGW (TAG2, "Warning message");
    ESP_LOGI (TAG2, "Info message");
    ESP_LOGE (TAG2, "Debug message");
    ESP_LOGV (TAG2, ARDUHAL_LOG_COLOR (ARDUHAL_LOG_COLOR_MAGENTA) "Verbose message");
    Serial.println ();
    esp_log_level_set (TAG, ESP_LOG_VERBOSE);
    ESP_LOGE (TAG, "Error message");
    ESP_LOGW (TAG, "Warning message");
    ESP_LOGI (TAG, "Info message");
    ESP_LOGD (TAG, "Debug message");
    ESP_LOGV (TAG, ARDUHAL_LOG_COLOR (ARDUHAL_LOG_COLOR_MAGENTA) "Verbose message");
    Serial.println ();
    //assert (giveNumber (0) != 0);
    // assert (giveNumber (-1) == 0);
    // assert (giveNumber (-2) == 0);
    // Serial.println ();
    esp_log_level_set (TAG, ESP_LOG_INFO);
    Serial.printf ("debug levels: %s:%d %s:%d\n", TAG, esp_log_level_get (TAG), TAG2, esp_log_level_get (TAG2));
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