#include <Arduino.h>
#include <M5StickCPlus.h>

constexpr auto LED = 10;

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto periodoLED = 1000;
constexpr auto ledEncendido = 50; // ms
constexpr auto ledApagado = 150; // ms

constexpr auto framerateLED = 20;
constexpr auto frametimeLED = 1000 / framerateLED;

constexpr auto periodoDisplay = 1000;
constexpr auto framerateDisplay = 15;
constexpr auto frametimeDisplay = 1000 / framerateDisplay;

/*
    El LED se enciende durante 10 ms dos veces, con otros 10 ms de espera entre cada uno. Se repite cada 1 segundo
*/

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

void parpadeaLED (void* pvParameters) {
    for (;;) {
        time_t start = millis (); // toma la referencia del inicio

        //--------------------- Procesado visual -------------------------------------------
        time_t ciclo = millis () % periodoLED;

        if ((ciclo > 0 && ciclo < ledEncendido) || (ciclo > ledEncendido + ledApagado && ciclo < ledEncendido * 2 + ledApagado)) {
            digitalWrite (LED, LOW);
        } else {
            digitalWrite (LED, HIGH);
        }
        //----------------------------------------------------------------------------------

        delay (frametimeLED - (millis () - start)); // Esperar el tiempo necesario para mantener fps
    }
}

void updateMessage (void* pvParameters) {
    for (;;) {
        time_t start = millis (); // toma la referencia del inicio
        static time_t lastDisplayUpdate = 0;

        //--------------------- Procesado visual -------------------------------------------
        if (millis () - lastDisplayUpdate >= periodoDisplay) {
            lastDisplayUpdate = millis ();
            Disbuff.fillSprite (BLUE);
            Disbuff.setCursor (10, 10);
            Disbuff.setTextColor (WHITE);
            Disbuff.setTextSize (7);
            uint seconds = millis () / 1000;
            uint minutes = seconds / 60;
            seconds = seconds % 60;
            Disbuff.printf ("%02d:%02d", minutes, seconds);
            Disbuff.setCursor (10, 65);
            Disbuff.setTextSize (3);
            Disbuff.printf ("%.2f V", M5.Axp.GetBatVoltage ());
            Disbuff.setCursor (10, 90);
            Disbuff.printf ("%.2f mA", M5.Axp.GetBatCurrent ());
            Disbuff.pushSprite (0, 0);
        }
        //----------------------------------------------------------------------------------
        vTaskDelay (frametimeDisplay - (millis () - start) / portTICK_PERIOD_MS); // Esperar el tiempo necesario para cumplir fps
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
    xTaskCreate (updateMessage, "Display", 2048, NULL, 1, &tareaMensaje);
}

void loop () {}