#ifndef OWM_ESP32_H
#define OWM_ESP32_H

#include <Arduino.h>
#include <HTTPClient.h>


constexpr auto TAG_MILIB = "OWM";
constexpr auto OWM_MIN_INTERVAL = 1000 * 60 * 5;

typedef struct {
    bool valid = false;
    float temperature = -200;
    float humidity = -1;
    float pressure = 0;
} WeatherData_t;

class OpenWeatherMapESP32 {
public:
    OpenWeatherMapESP32();
    ~OpenWeatherMapESP32 ();
    void begin (String apikey, String city);
    float getTemperature ();
    float getHumidity ();
    float getPressure ();
    bool askForWeather ();
    bool isValid ();

protected:
    String _apiKey;
    HTTPClient _client;
    WeatherData_t _weatherData;
    String _city;
    time_t _lastOWMRequest = -OWM_MIN_INTERVAL;
};



#endif // OWM_ESP32_H