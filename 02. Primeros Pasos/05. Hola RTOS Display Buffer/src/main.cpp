#include <Arduino.h>
//#include <M5StickCPlus.h>
#include <I2C_AXP192.h>
#include <TFT_eSPI.h>
#include "driver/ledc.h"

TaskHandle_t tareaLED = NULL;
TaskHandle_t tareaMensaje = NULL;

constexpr int LEDC_BASE_FREQ = 5000;
constexpr int LEDC_RESOLUTION = LEDC_TIMER_8_BIT;

constexpr auto esperaLED = 30;
constexpr auto esperaMensaje = 1000;

bool sendMessage = false;

I2C_AXP192 axp192 (I2C_AXP192_DEFAULT_ADDRESS, Wire1);
TFT_eSPI lcd = TFT_eSPI ();

TFT_eSprite Disbuff = TFT_eSprite (&lcd);

void updateDisplay () {
    Disbuff.fillSprite (TFT_BLACK);
    Disbuff.setCursor (10, 10);
    Disbuff.setTextColor (TFT_WHITE);
    Disbuff.setTextSize (7);
    uint seconds = millis () / 1000;
    uint minutes = seconds / 60;
    seconds = seconds % 60;
    Disbuff.printf ("%02d:%02d", minutes, seconds);
    Disbuff.setCursor (10, 65);
    Disbuff.setTextSize (3);
    Disbuff.printf ("%.2f V", axp192.getBatteryVoltage ());
    Disbuff.setCursor (10, 90);
    float batteryCurrent = axp192.getBatteryDischargeCurrent ();
    if (batteryCurrent <= 0.01) {
        batteryCurrent = -axp192.getBatteryChargeCurrent ();
    }
    Disbuff.printf ("%.2f mA", batteryCurrent);
    Disbuff.pushSprite (0, 0);
}


void ledcAnalogWrite (uint8_t canal, uint32_t valor, uint32_t valorMaximo = 255) {
  // calcular brillo, 4095 desde 2 ^ 12 - 1
    static uint32_t max_range = pow (2, LEDC_RESOLUTION) - 1;
    uint32_t brillo = (max_range / valorMaximo) * min (valor, valorMaximo);
    // Serial.println (max_range - brillo);

    // escribir brillo a LEDC
    ledcWrite (canal, brillo);
}

void parpadeaLED (void* pvParameters) {
    for (;;) {
        static int brilloLED = 0;    // how bright the LED is
        static int cambioBrillo = 5;    // how many points to fade the LED by
        int maximo = 255;
        ledcAnalogWrite (LEDC_CHANNEL_0, brilloLED, maximo);
        brilloLED = brilloLED + cambioBrillo;
        if (brilloLED <= 0 || brilloLED >= maximo) {
            cambioBrillo = -cambioBrillo;
        }
        vTaskDelay (pdMS_TO_TICKS (esperaLED));
    }
}


// void parpadeaLED (void* pvParameters) {
//     for (;;)   {
//         digitalWrite (BUILTIN_LED, HIGH);
//         delay (esperaLED);
//         digitalWrite (BUILTIN_LED, LOW);
//         delay (esperaLED);
//     }
// }

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
    ledcSetup (LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin (BUILTIN_LED, LEDC_CHANNEL_0);

    lcd.init ();
    lcd.setRotation (1);
    lcd.fillScreen (TFT_BLACK);
    Disbuff.createSprite (TFT_HEIGHT, TFT_WIDTH);
    Disbuff.setRotation (1);
    delay (100);
    xTaskCreate (parpadeaLED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, &tareaLED);
    xTaskCreate (escribeMensaje, "Mensaje", 2048, NULL, 1, &tareaMensaje);
}

void loop () {
}