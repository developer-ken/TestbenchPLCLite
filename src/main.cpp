#include <Arduino.h>
#include <esp_heap_caps.h>

#include "pinout.h"
#include "display.h"
#include "knob.h"

void setup()
{
    pinMode(PIN_BEEP, OUTPUT);
    pinMode(PIN_DO1, OUTPUT);
    pinMode(PIN_DO2, OUTPUT);
    pinMode(PIN_DO3, OUTPUT);
    pinMode(PIN_DO4, OUTPUT);
    analogWrite(PIN_BEEP, 0); // Off
    analogWriteFrequency(PIN_BEEP, 4000);
    Serial.begin(115200);

    init_knob();
    lvsetup();
}

void loop()
{
    lvloop();
    vTaskDelay(0);
}