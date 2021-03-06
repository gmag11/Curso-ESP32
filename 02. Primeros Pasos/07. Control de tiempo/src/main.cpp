#include <Arduino.h>
#include <M5StickCPlus.h>

constexpr auto LED = 10;

//TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaLED = NULL;
TimerHandle_t tareaDisplay = NULL;

constexpr auto periodoDisplay = 10;
constexpr auto framerateDisplay = 15;
constexpr auto frametimeDisplay = 1000 / framerateDisplay;
/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

void updateDisplay (void* pvParameters = NULL) {
    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;

    if (millis () - lastMeasurement > 1000) {
        lastMeasurement = millis ();
        current = M5.Axp.GetBatCurrent ();
        voltage = M5.Axp.GetBatVoltage ();
    }

    if (millis () - lastDisplayUpdate > periodoDisplay) {
        lastDisplayUpdate = millis ();
        Disbuff.fillSprite (BLACK);
        Disbuff.setCursor (10, 10);
        Disbuff.setTextColor (WHITE);
        Disbuff.setTextSize (4);
        time_t currentTime = millis ();
        uint cents = (currentTime % 1000) / 10;
        uint seconds = currentTime / 1000;
        uint minutes = seconds / 60;
        seconds = seconds % 60;
        Disbuff.printf ("%02d:%02d.%02d", minutes, seconds, cents);
        Disbuff.setCursor (10, 55);
        Disbuff.setTextSize (2);
        Disbuff.printf ("%.2f V %.2f mA", voltage, current);
        //Disbuff.setCursor (10, 90);
        Disbuff.pushSprite (0, 0);
    }

}

void parpadeaLED (void* pvParameters) {
    const int letFlashes = 2;
    const int ledONtime = 40;
    const int ledOFFtime = 100;
    const int ledCyclePeriod = 1000;

    static time_t lastLEDCycle = 0;

    for (;;) {
        if (millis () - lastLEDCycle > ledCyclePeriod) {
            lastLEDCycle = millis ();
            for (int i = 0; i < letFlashes; i++) {
                digitalWrite (LED, LOW);
                delay (ledONtime);
                digitalWrite (LED, HIGH);
                delay (ledOFFtime);
            }
        }
        delay (10);
    }
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    M5.begin ();
    M5.Lcd.setRotation (3);
    Disbuff.createSprite (240, 135);
    Disbuff.setRotation (3);
    Disbuff.fillSprite (BLUE);
    Disbuff.setTextColor (WHITE);
    Disbuff.setTextSize (2);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("Hello world!");
    Disbuff.pushSprite (0, 0);
    delay (500);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    tareaDisplay = xTimerCreate ("Display", 40 / portTICK_PERIOD_MS, pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}