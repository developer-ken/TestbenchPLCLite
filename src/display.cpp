#include <Arduino.h>
#include "Arduino_GFX_Library.h"

#include "display.h"
#include "pinout.h"
#include "knob.h"

#include "gui_guider.h"
#include "events_init.h"

Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(PIN_TFT_RS, PIN_TFT_CS, PIN_TFT_CLK, PIN_TFT_MOSI, -1, 1);
Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_TFT_RST, 0, true, TFT_VER_RES, TFT_HOR_RES, 0, 40, 53, 0);

uint32_t draw_bufA[DRAW_BUF_SIZE / 4];
uint32_t draw_bufB[DRAW_BUF_SIZE / 4];

lv_ui guider_ui;

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    lv_draw_sw_rgb565_swap(px_map, w * h);
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
    lv_display_flush_ready(disp); /* tell lvgl that flushing is done */
}

void my_knob_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    data->enc_diff = knobdelta;
    knobdelta = 0; // data->enc_diff;
    if (digitalRead(PIN_KNOB_PRESS) == LOW)
    {
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

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

    lv_init();
    lv_tick_set_cb(my_tick);

    lv_display_t *disp;
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_bufA, draw_bufB, sizeof(draw_bufA), LV_DISPLAY_RENDER_MODE_FULL);

    lv_indev_t *knob = lv_indev_create();
    lv_indev_set_type(knob, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(knob, my_knob_read);

    setup_ui(&guider_ui);
    events_init(&guider_ui);

    lv_group_t *g1 = lv_group_create();
    lv_indev_set_group(knob, g1);
    lv_group_add_obj(g1, guider_ui.screen_fileselect);
    lv_group_add_obj(g1, guider_ui.screen_run);
    lv_group_add_obj(g1, guider_ui.screen_stath);
    lv_group_add_obj(g1, guider_ui.screen_nonvolatile);
    lv_group_add_obj(g1, guider_ui.screen_autoboot);
    lv_group_add_obj(g1, guider_ui.screen_staterecover);
    lv_group_add_obj(g1, guider_ui.screen_wireless);
    
    const char* screen_wireless_connqr_data = "WIFI:T:WPA/WPA2;S:testbenchplc-2n75;P:43567834;H:true;";
    lv_qrcode_update(guider_ui.screen_wireless_connqr, screen_wireless_connqr_data, lv_strlen(screen_wireless_connqr_data));

    lv_timer_handler();
    digitalWrite(PIN_TFT_BL, HIGH); // 打开背光
}

void lvloop()
{
    lv_timer_handler(); /* let the GUI do its work */
}
