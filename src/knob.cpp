#include <Arduino.h>
#include "pinout.h"

float_t knobdelta;
uint32_t last_knob_update = 0;

void knob_update()
{
    uint32_t now = millis();
    if (now - last_knob_update < 2)
    {
        return; // 避免抖动
    }
    int8_t stateA = digitalRead(PIN_KNOB_A),
           stateB = digitalRead(PIN_KNOB_B);
    float_t turnratio = 1.00;
    if (now - last_knob_update > 250)
    {
        knobdelta = 0;
    }
    else if (now - last_knob_update < 50)
    {
        turnratio = 0.02 * (50 - now + last_knob_update) + 1; // 如果旋转速度过快，增加增量
        turnratio *= abs(turnratio);                          // 增加非线性
    }
    last_knob_update = now;
    if (stateA ^ stateB)
    {
        knobdelta += turnratio; // 顺时针
    }
    else
    {
        knobdelta -= turnratio; // 逆时针
    }
}

void init_knob()
{
    pinMode(PIN_KNOB_A, INPUT_PULLUP);
    pinMode(PIN_KNOB_B, INPUT_PULLUP);
    pinMode(PIN_KNOB_PRESS, INPUT_PULLUP);
    attachInterrupt(PIN_KNOB_A, knob_update, CHANGE);
}