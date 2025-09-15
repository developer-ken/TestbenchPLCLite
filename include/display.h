#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <lvgl.h>
#include <SPI.h>

#define TFT_HOR_RES   240
#define TFT_VER_RES   135
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES  * (LV_COLOR_DEPTH / 8))

struct GConfig
{
    bool NonVolatile,
        AutoBoot,
        StateRecover,
        Wifi;

    char SelectedProgram[64];
};

void lvsetup();
void lvloop();
void lvproglistupdate();

#endif