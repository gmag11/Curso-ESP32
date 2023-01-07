#include <Arduino.h>
#include <WiFi.h>
//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>

#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
constexpr auto SSID = "SSID";
constexpr auto PASSWORD = "PASSWORD";
#endif

// constexpr auto LED = 5;
constexpr auto LED = 10;
constexpr auto LED_ON = LOW;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;
TimerHandle_t tareaDisplay = NULL;

/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);

void updateDisplay (void* pvParameters) {
    constexpr auto periodoDisplay = 10;

    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;
    static String ssid;
    static String localip;
    static int numStations;

    if (millis () - lastMeasurement >= 1000) {
        lastMeasurement = millis ();
        current = axp192.getBatteryDischargeCurrent ();
        if (current <= 0.01) {
            current = -axp192.getBatteryChargeCurrent ();
        }
        voltage = axp192.getBatteryVoltage ();
        ssid = WiFi.softAPSSID ();
        localip = WiFi.softAPIP ().toString ();
        numStations = WiFi.softAPgetStationNum ();
    }

        //--------------------- Procesado visual -------------------------------------------
    if (millis () - lastDisplayUpdate >= periodoDisplay) {
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
        Disbuff.setCursor (10, 50);
        Disbuff.setTextSize (2);
        Disbuff.setTextColor (TFT_RED);
        Disbuff.printf ("%.2f V %.2f mA", voltage, current);
        Disbuff.setCursor (10, 75);
        Disbuff.setTextColor (TFT_WHITE);
        Disbuff.printf ("%d: %s", numStations, ssid.c_str ());
        Disbuff.setCursor (10, 100);
        Disbuff.printf ("IP: %s", localip.c_str ());
        Disbuff.pushSprite (0, 0);
    }
    //----------------------------------------------------------------------------------
}

void parpadeaLED (void* pvParameters) {
    const int letFlashes = 2;
    const int ledONtime = 40;
    const int ledOFFtime = 100;
    const int ledCyclePeriod = 5000;

    static time_t lastLEDCycle = 0;

    for (;;) {
        if (millis () - lastLEDCycle >= ledCyclePeriod) {
            lastLEDCycle = millis ();
            for (int i = 0; i < letFlashes; i++) {
                digitalWrite (LED, LED_ON);
                delay (ledONtime);
                digitalWrite (LED, !LED_ON);
                delay (ledOFFtime);
            }
        }
        delay (10);
    }
}

void newStationConnected (arduino_event_t* event) {
    if (event->event_id == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        Serial.print ("\nNueva estacion conectada.\n");
        Serial.printf ("MAC: " MACSTR "\n", MAC2STR (event->event_info.wifi_ap_staconnected.mac));
    }
}

void newStationGotIP (arduino_event_t* event) {
    if (event->event_id == ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED) {
        Serial.printf ("IP: %s\n", IPAddress (event->event_info.wifi_ap_staipassigned.ip.addr).toString ().c_str ());
    }
}

const int CHANNEL = 11;

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    WiFi.mode (WIFI_AP);
    WiFi.softAP (SSID, PASSWORD, CHANNEL);
    WiFi.onEvent (newStationConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    WiFi.onEvent (newStationGotIP, ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);

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
    Disbuff.fillSprite (TFT_BLACK);
    Disbuff.setTextColor (TFT_WHITE);
    Disbuff.setTextSize (2);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("Conectando");
    Disbuff.pushSprite (0, 0);
    delay (100);

    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);

    const uint fps = 25;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS(1000/fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {}