#include <Arduino.h>
//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>

constexpr auto LED = 10;

//TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaLED = NULL;
TimerHandle_t tareaDisplay = NULL;

constexpr auto periodoDisplay = 10;
constexpr auto framerateDisplay = 25;
constexpr auto frametimeDisplay = 1000 / framerateDisplay;

/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);

void updateDisplay (void* pvParameters = NULL) {
    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;

    if (millis () - lastMeasurement > 1000) {
        lastMeasurement = millis ();
        current = axp192.getBatteryDischargeCurrent ();
        if (current <= 0.01) {
            current = -axp192.getBatteryChargeCurrent ();
        }
        voltage = axp192.getBatteryVoltage ();
    }

    if (millis () - lastDisplayUpdate > frametimeDisplay) {
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
    const int letFlashes = 2;
    const int ledONtime = 40;
    const int ledOFFtime = 100;
    const int ledCyclePeriod = 5000;

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
    digitalWrite (LED, HIGH);
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
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    tareaDisplay = xTimerCreate ("Display", 40 / portTICK_PERIOD_MS, pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}