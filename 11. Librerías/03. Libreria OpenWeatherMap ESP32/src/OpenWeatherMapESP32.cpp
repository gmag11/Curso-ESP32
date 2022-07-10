#include "OpenWeatherMapESP32.h"
#include <QuickDebug.h>
#include <ArduinoJson.h>

constexpr auto OWM_URL = "http://api.openweathermap.org/data/2.5/weather";

OpenWeatherMapESP32::OpenWeatherMapESP32 () {}
OpenWeatherMapESP32::~OpenWeatherMapESP32 () {}

void OpenWeatherMapESP32::begin (String apikey, String city) {
    _apiKey = apikey;
    _city = city;
    DEBUG_INFO (TAG_MILIB, "OWM API Key: %s", _apiKey.c_str ());
}

bool OpenWeatherMapESP32::askForWeather () {
    if (!_city.length () || !_apiKey.length ()) {
        DEBUG_ERROR (TAG_MILIB, "No se ha inicializado el OWM. Ciudad: %s, API Key: %s", _city.c_str (), _apiKey.c_str ());
        return false;
    }

    String url = String (OWM_URL) + "?units=metric&q=" + _city + "&appid=" + _apiKey;

    if (millis () - _lastOWMRequest < OWM_MIN_INTERVAL) {
        DEBUG_INFO (TAG_MILIB, "OWM request too soon. Valid data: %s", _weatherData.valid ? "true" : "false");
        DEBUG_INFO (TAG_MILIB, "Remaining time: %d", OWM_MIN_INTERVAL - (millis () - _lastOWMRequest));
        return _weatherData.valid;
    }

    _client.begin (url);
    int httpCode = _client.GET ();
    if (httpCode == 200) {
        _lastOWMRequest = millis ();
        
        StaticJsonDocument<64> filter;

        JsonObject filter_main = filter.createNestedObject ("main");
        filter_main["temp"] = true;
        filter_main["pressure"] = true;
        filter_main["humidity"] = true;

        String payload = _client.getString ();
        DEBUG_INFO (TAG_MILIB, "%s", payload.c_str ());

        StaticJsonDocument<128> doc;

        DeserializationError error = deserializeJson (doc, payload, DeserializationOption::Filter (filter));

        if (error) {
            Serial.print ("deserializeJson() failed: ");
            Serial.println (error.c_str ());
            _weatherData.valid = false;
            return false;
        }
        
        JsonObject main = doc["main"];
        _weatherData.temperature = main["temp"].as<float> ();
        _weatherData.humidity = main["humidity"].as<float> ();
        _weatherData.pressure = main["pressure"].as<float> ();
        _weatherData.valid = true;

        DEBUG_INFO (TAG_MILIB, "Temperature: %.2f ºC", _weatherData.temperature);
        DEBUG_INFO (TAG_MILIB, "Humidity: %.0f %%", _weatherData.humidity);
        DEBUG_INFO (TAG_MILIB, "Pressure: %.0f hpa", _weatherData.pressure);
    } else {
        DEBUG_ERROR (TAG_MILIB, "Error: %d %s", httpCode, _client.errorToString (httpCode).c_str ());
        _weatherData.temperature = -200;
        _weatherData.humidity = -1;
        _weatherData.pressure = 0;
        _weatherData.valid = false;
        return false;
    }

    return true;
}

float OpenWeatherMapESP32::getTemperature () { return _weatherData.temperature; }
float OpenWeatherMapESP32::getHumidity () { return _weatherData.humidity; }
float OpenWeatherMapESP32::getPressure () { return _weatherData.pressure; }

bool OpenWeatherMapESP32::isValid () { return _weatherData.valid; }