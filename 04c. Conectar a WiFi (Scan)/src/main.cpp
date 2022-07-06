#include <Arduino.h>
#include <WiFi.h>
#include <M5StickCPlus.h>

#if __has_include("wificonfig.h")
#include "wificonfig.h"
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

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

int numRedes = 0;

void updateDisplay (void* pvParameters) {
    constexpr auto periodoDisplay = 10;

    static time_t lastDisplayUpdate = 0;
    static time_t lastMeasurement = 0;
    static float current;
    static float voltage;

    if (millis () - lastMeasurement > 1000) {
        lastMeasurement = millis ();
        current = M5.Axp.GetBatCurrent ();
        voltage = M5.Axp.GetBatVoltage ();
    }

        //--------------------- Procesado visual -------------------------------------------
    if (millis () - lastDisplayUpdate >= periodoDisplay) {
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
        Disbuff.setCursor (10, 50);
        Disbuff.setTextSize (2);
        Disbuff.setTextColor (RED);
        Disbuff.printf ("%.2f V %.2f mA", voltage, current);
        Disbuff.setCursor (10, 75);
        Disbuff.setTextColor (WHITE);
        Disbuff.printf ("Redes: %d", numRedes);
        Disbuff.pushSprite (0, 0);
    }
    //----------------------------------------------------------------------------------
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

void setup () {
    Serial.begin (115200);
    pinMode (LED, OUTPUT);
    digitalWrite (LED, !LED_ON);
    WiFi.mode (WIFI_STA);
    WiFi.disconnect ();

    M5.begin ();
    M5.Lcd.setRotation (3);
    Disbuff.createSprite (240, 135);
    Disbuff.setRotation (3);
    Disbuff.fillSprite (BLACK);
    Disbuff.setTextColor (WHITE);
    Disbuff.setTextSize (2);
    Disbuff.setCursor (10, 10);
    Disbuff.printf ("Conectando");
    Disbuff.pushSprite (0, 0);
    delay (100);

    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);

    const uint fps = 50;

    tareaDisplay = xTimerCreate ("Display", pdMS_TO_TICKS(1000/fps), pdTRUE, NULL, updateDisplay);
    xTimerStart (tareaDisplay, 0);
}

void loop () {
    
    numRedes = WiFi.scanNetworks ();
    Serial.println ("Escaneo finalizado");
    if (numRedes == 0) {
        Serial.println ("No se han encontrado redes");
    } else {
        Serial.printf ("%d redes encontradas\n", numRedes);
        for (int i = 0; i < numRedes; ++i) {
            // Print SSID and RSSI for each network found
            Serial.printf ("%d: %s (%d)%s\n",
                           i + 1,
                           WiFi.SSID (i).c_str (),
                           WiFi.RSSI (i),
                           WiFi.encryptionType (i) == WIFI_AUTH_OPEN ? " " : "*"
                           );
            delay (10);
        }
    }
    Serial.println ();

    // Wait a bit before scanning again
    delay (5000);
}