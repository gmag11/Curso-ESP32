#include <Arduino.h>
//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>
#include "driver/ledc.h"

constexpr auto LED = 10;

//TaskHandle_t tareaLED = NULL;
TimerHandle_t tareaLED = NULL;
TimerHandle_t tareaDisplay = NULL;

constexpr int LEDC_BASE_FREQ = 5000;
constexpr int LEDC_RESOLUTION = LEDC_TIMER_8_BIT;

constexpr auto periodoLED = 1000;
constexpr auto ledEncendido = 50; // ms
constexpr auto ledApagado = 150; // ms

constexpr auto framerateLED = 20;
constexpr auto frametimeLED = 1000 / framerateLED;

constexpr auto periodoDisplay = 10; // Cada cuánto tiempo se comprueba si hay actualizaciones de pantalla
constexpr auto framerateDisplay = 25; // Cuadros por segundo
constexpr auto frametimeDisplay = 1000 / framerateDisplay;

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);

void updateDisplay (void* pvParameters = NULL) {
    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;

    if (millis()- lastMeasurement >= 1000) {
        lastMeasurement = millis();
        current = axp192.getBatteryDischargeCurrent ();
        if (current <= 0.01) {
            current = -axp192.getBatteryChargeCurrent ();
        }
        voltage = axp192.getBatteryVoltage ();
    }

    if (millis () - lastDisplayUpdate >= frametimeDisplay) {
        lastDisplayUpdate = millis ();
        Disbuff.fillSprite (TFT_BLACK);
        Disbuff.setCursor (10, 10);
        Disbuff.setTextColor (TFT_WHITE);
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
    digitalWrite (LED, !digitalRead (LED));
}

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);

    Wire1.begin (21, 22);
    Wire1.setClock (400000);

    I2C_AXP192_InitDef initDef = {
        .EXTEN = true,
        .BACKUP = true,
        .DCDC1 = 3300,
        .DCDC2 = 0,
        .DCDC3 = 0,
        .LDO2 = 2800,
        .LDO3 = 3000,
        .GPIO0 = 2800,
        .GPIO1 = -1,
        .GPIO2 = -1,
        .GPIO3 = -1,
        .GPIO4 = -1,
    };
    axp192.begin (initDef);
    lcd.init ();
    lcd.setRotation (1);
    lcd.fillScreen (TFT_BLACK);
    Disbuff.createSprite (TFT_HEIGHT, TFT_WIDTH);
    Disbuff.setRotation (1);
    Disbuff.fillSprite (TFT_BLUE);
    Disbuff.setTextColor (TFT_WHITE);
    Disbuff.setTextSize (2);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("Hello world!");
    Disbuff.pushSprite (0, 0);
    delay (500);
    //xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);

    tareaLED = xTimerCreate ("LED", periodoLED / portTICK_PERIOD_MS, pdTRUE, NULL, parpadeaLED);
    xTimerStart (tareaLED, 0);

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS (periodoDisplay), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}