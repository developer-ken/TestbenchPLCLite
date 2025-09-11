#include <Arduino.h>
#include "pinout.h"

int16_t knobdelta;
uint32_t last_knob_update = 0;
bool locknlock = false;
uint8_t rotatestate = 0;

void knob_isr()
{
    last_knob_update = millis();
    locknlock = true;
}

void init_knob()
{
    pinMode(PIN_KNOB_A, INPUT);
    pinMode(PIN_KNOB_B, INPUT);
    pinMode(PIN_KNOB_PRESS, INPUT);
    attachInterrupt(PIN_KNOB_A, knob_isr, CHANGE);
    xTaskCreate(
        [](void *param)
        {
            for (;;)
            {
                if (locknlock && (millis() - last_knob_update > 2))
                {
                    locknlock = false;
                    bool a = digitalRead(PIN_KNOB_A);
                    bool b = digitalRead(PIN_KNOB_B);
                    //Serial.printf("A=%d B=%d r=%d\n", a, b, rotatestate);
                    if (rotatestate == 0)
                    {
                        if (a == b)
                        {
                            rotatestate = 1;
                        }
                        else
                        {
                            rotatestate = 2;
                        }
                    }
                    else
                    {
                        if (a == b && rotatestate == 1)
                        {
                            knobdelta--;
                        }
                        else if (a != b && rotatestate == 2)
                        {
                            knobdelta++;
                        }
                        rotatestate = 0;
                    }
                }
                else if ((millis() - last_knob_update > 150))
                {
                    rotatestate = 0;
                }
                vTaskDelay(1);
            }
        },
        "knobtask",
        2048,
        NULL,
        1,
        NULL);
}