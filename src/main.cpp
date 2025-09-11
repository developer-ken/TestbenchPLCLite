#include <Arduino.h>
#include <esp_heap_caps.h>

#include "pinout.h"
#include "display.h"
#include "knob.h"

uint16_t *dataregisterpool;

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
    dataregisterpool = (uint16_t *)malloc(sizeof(uint16_t) * 524288); // 1MB for data registers
    init_knob();
    lvsetup();
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.printf("Reset reason: %d\n", reason);
}

void loop()
{
    lvloop();
    vTaskDelay(0);
}