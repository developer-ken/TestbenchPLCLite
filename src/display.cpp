#include <Arduino.h>
#include "Arduino_GFX_Library.h"

#include "display.h"
#include "pinout.h"
#include "knob.h"


Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(PIN_TFT_RS, PIN_TFT_CS, PIN_TFT_CLK, PIN_TFT_MOSI, -1, 2);
Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_TFT_RST, 0, true, TFT_VER_RES, TFT_HOR_RES, 0, 40, 53, 0);

void lvsetup()
{
    pinMode(PIN_TFT_BL, OUTPUT);
    pinMode(PIN_TFT_RS, OUTPUT);
    pinMode(PIN_TFT_CS, OUTPUT);
    pinMode(PIN_TFT_CLK, OUTPUT);
    pinMode(PIN_TFT_MOSI, OUTPUT);
    pinMode(PIN_TFT_RST, OUTPUT);
    gfx->begin(80000000L);
    gfx->setRotation(1);
    gfx->invertDisplay(false);

    analogWrite(PIN_TFT_BL, 255); // 打开背光
}

void lvloop()
{
    
}