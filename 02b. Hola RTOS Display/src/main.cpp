#include <Arduino.h>
#include <M5StickCPlus.h>

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr auto esperaLED = 300;
constexpr auto esperaMensaje = 1000;
int counter = 0;
bool sendMessage = false;

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
        sendMessage = true;
        delay (esperaMensaje);
    }
}

void setup () {
    Serial.begin (115200);
    M5.begin ();
    pinMode (BUILTIN_LED, OUTPUT);
    M5.Lcd.setRotation (3);
    M5.Lcd.fillScreen (BLACK);
    delay (100);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", configMINIMAL_STACK_SIZE, NULL, 1, &tareaMensaje);
}

void updateDisplay () {
    M5.Lcd.fillScreen (BLACK);
    M5.Lcd.setCursor (10, 10);
    M5.Lcd.setTextColor (WHITE);
    M5.Lcd.setTextSize (7);
    uint seconds = millis () / 1000;
    uint minutes = seconds / 60;
    seconds = seconds % 60;
    M5.Lcd.printf ("%02d:%02d", minutes, seconds, counter);
    M5.Lcd.setCursor (10, 65);
    M5.Lcd.setTextSize (3);
    M5.Lcd.printf ("%.2f V", M5.Axp.GetBatVoltage ());
    M5.Lcd.setCursor (10, 90);
    M5.Lcd.printf ("%.2f mA", M5.Axp.GetBatCurrent ());
}

void loop () {
    if (sendMessage) {
        sendMessage = false;
        //Serial.printf ("Hola mundo: %d\n", counter);
        updateDisplay ();
        //counter++;
    }
}