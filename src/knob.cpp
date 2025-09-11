#include <Arduino.h>
#include "pinout.h"

int16_t knobdelta;
uint32_t last_knob_update = 0;
bool locknlock = false;

void knob_isr()
{
    uint32_t now = millis();
    if (now - last_knob_update < 5)
    {
        last_knob_update = now;
        return;
    }
    last_knob_update = now;
    bool a = digitalRead(PIN_KNOB_A);
    bool b = digitalRead(PIN_KNOB_B);
    if (!a)
    {
        if (a == b)
        {
            knobdelta--;
        }
        else
        {
            knobdelta++;
        }
    }
}

void init_knob()
{
    pinMode(PIN_KNOB_A, INPUT);
    pinMode(PIN_KNOB_B, INPUT);
    pinMode(PIN_KNOB_PRESS, INPUT);
    attachInterrupt(PIN_KNOB_A, knob_isr, FALLING);
}