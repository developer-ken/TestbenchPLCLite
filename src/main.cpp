#include <Arduino.h>
#include <esp_heap_caps.h>

#include "pinout.h"
#include "display.h"
#include "knob.h"

uint16_t *dataregisterpool;

void setup()
{
    pinMode(PIN_BROWNOUT, INPUT_PULLUP);
    pinMode(PIN_LEDPWR, OUTPUT);
    pinMode(PIN_LEDHB, OUTPUT);
    pinMode(PIN_BEEP, OUTPUT);
    digitalWrite(PIN_LEDHB, HIGH);
    digitalWrite(PIN_LEDPWR, LOW);
    while (digitalRead(PIN_BROWNOUT) == LOW)
    {
        vTaskDelay(100);
        digitalWrite(PIN_LEDPWR, HIGH);
    }
    analogWrite(PIN_BEEP, 0); // Off
    analogWriteFrequency(PIN_BEEP, 4000);
    Serial.begin(115200);
    dataregisterpool = (uint16_t *)malloc(sizeof(uint16_t) * 524288); // 1MB for data registers
    init_knob();
    lvsetup();
}

void loop()
{
    lvloop();
    vTaskDelay(0);
}