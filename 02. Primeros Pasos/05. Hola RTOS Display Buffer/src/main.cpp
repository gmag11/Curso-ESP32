#include <Arduino.h>
#include <M5StickCPlus.h>

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto esperaLED = 300;
constexpr auto esperaMensaje = 1000;
int counter = 0;
bool sendMessage = false;

TFT_eSprite Disbuff = TFT_eSprite (&M5.Lcd);

void updateDisplay () {
    Disbuff.fillSprite (BLACK);
    Disbuff.setCursor (10, 10);
    Disbuff.setTextColor (WHITE);
    Disbuff.setTextSize (7);
    uint seconds = millis () / 1000;
    uint minutes = seconds / 60;
    seconds = seconds % 60;
    Disbuff.printf ("%02d:%02d", minutes, seconds, counter);
    Disbuff.setCursor (10, 65);
    Disbuff.setTextSize (3);
    Disbuff.printf ("%.2f V", M5.Axp.GetBatVoltage ());
    Disbuff.setCursor (10, 90);
    Disbuff.printf ("%.2f mA", M5.Axp.GetBatCurrent ());
    Disbuff.pushSprite (0, 0);
}


void parpadeaLED (void* pvParameters) {
    for (;;)   {
        digitalWrite (BUILTIN_LED, HIGH);
        delay (esperaLED);
        digitalWrite (BUILTIN_LED, LOW);
        delay (esperaLED);
    }
}

void escribeMensaje (void* pvParameters) {
    for (;;) {
        static time_t last = 0;
        if (millis () - last > esperaMensaje) {
            last = millis ();
            updateDisplay ();
        }
    }
}

void setup () {
    Serial.begin (115200);
    M5.begin ();
    pinMode (BUILTIN_LED, OUTPUT);
    M5.Lcd.setRotation (3);
    Disbuff.createSprite (240, 135);
    Disbuff.setRotation (3);
    delay (100);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}