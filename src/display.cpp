#include <Arduino.h>
#include <esp_sleep.h>
#include "Arduino_GFX_Library.h"

#include "display.h"
#include "pinout.h"
#include "knob.h"
#include "storage.h"
#include "server.h"

#include "gui_guider.h"
#include "events_init.h"

Arduino_DataBus *bus = new Arduino_ESP32SPIDMA(PIN_TFT_RS, PIN_TFT_CS, PIN_TFT_CLK, PIN_TFT_MOSI, -1, 2);
Arduino_GFX *gfx = new Arduino_ST7789(bus, PIN_TFT_RST, 0, true, TFT_VER_RES, TFT_HOR_RES, 0, 40, 53, 0);

uint32_t draw_bufA[DRAW_BUF_SIZE / 4];

bool powerloss = false;

lv_ui guider_ui;

extern struct GConfig config;

void SaveConfig();
void TryRunSelected();
void RunStateSwitch();

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

void WifiStateUpdate()
{
    if (lv_obj_has_state(guider_ui.screen_wireless, LV_STATE_CHECKED))
    {
        WifiEnable();
        String connstring = "WIFI:T:WPA2;S:" + ssid + ";P:" + password + ";H:true;";
        const char *screen_wireless_connqr_data = connstring.c_str();
        lv_qrcode_set_dark_color(guider_ui.screen_wireless_connqr,
                                 lv_color_hex(0x000000));
        lv_qrcode_update(guider_ui.screen_wireless_connqr,
                         screen_wireless_connqr_data,
                         lv_strlen(screen_wireless_connqr_data));
    }
    else
    {
        WifiDisable();
        const char *screen_wireless_connqr_data = " ";
        lv_qrcode_set_dark_color(guider_ui.screen_wireless_connqr,
                                 lv_color_hex(0xF0F0F0));
        lv_qrcode_update(guider_ui.screen_wireless_connqr,
                         screen_wireless_connqr_data,
                         lv_strlen(screen_wireless_connqr_data));
    }
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
    lv_display_set_buffers(disp, draw_bufA, NULL, sizeof(draw_bufA), LV_DISPLAY_RENDER_MODE_PARTIAL);

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
    lv_group_set_default(g1);

    // 为screen_wireless开关添加事件处理
    lv_obj_add_event_cb(guider_ui.screen_wireless, [](lv_event_t *e)
                        { WifiStateUpdate(); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_autoboot, [](lv_event_t *e)
                        { config.AutoBoot = lv_obj_has_state(guider_ui.screen_autoboot, LV_STATE_CHECKED); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_nonvolatile, [](lv_event_t *e)
                        { config.NonVolatile = lv_obj_has_state(guider_ui.screen_nonvolatile, LV_STATE_CHECKED); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_staterecover, [](lv_event_t *e)
                        { config.StateRecover = lv_obj_has_state(guider_ui.screen_staterecover, LV_STATE_CHECKED); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_wireless, [](lv_event_t *e)
                        { config.Wifi = lv_obj_has_state(guider_ui.screen_wireless, LV_STATE_CHECKED); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_run, [](lv_event_t *e)
                        { config.Running = lv_obj_has_state(guider_ui.screen_run, LV_STATE_CHECKED); RunStateSwitch(); }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_add_event_cb(guider_ui.screen_fileselect, [](lv_event_t *e)
                        { config.SelectedProgramIndex = lv_roller_get_selected(guider_ui.screen_fileselect); }, LV_EVENT_VALUE_CHANGED, NULL);

    // 根据config设置guiderui的各个switch状态
    lv_obj_set_state(guider_ui.screen_nonvolatile, LV_STATE_CHECKED, config.NonVolatile);
    lv_obj_set_state(guider_ui.screen_autoboot, LV_STATE_CHECKED, config.AutoBoot);
    lv_obj_set_state(guider_ui.screen_staterecover, LV_STATE_CHECKED, config.StateRecover);
    lv_obj_set_state(guider_ui.screen_wireless, LV_STATE_CHECKED, config.Wifi);

    lvproglistupdate();

    config.Running &= config.AutoBoot;

    if (config.SelectedProgramIndex < lv_roller_get_option_count(guider_ui.screen_fileselect))
    {
        lv_roller_set_selected(guider_ui.screen_fileselect, config.SelectedProgramIndex, LV_ANIM_OFF);
        if (config.Running)
        {
            lv_obj_set_state(guider_ui.screen_run, LV_STATE_CHECKED, true);
        }
    }

    WifiStateUpdate();

    lv_timer_handler();
    analogWrite(PIN_TFT_BL, 255); // 打开背光
    RunStateSwitch();
}

void lvloop()
{
    int d = digitalRead(PIN_BROWNOUT);
    static uint32_t powerfailtime = millis();
    static uint32_t dimmtime = millis();
    if (!powerloss && !d)
    {
        // Brownout detected
        lv_obj_remove_flag(guider_ui.screen_powerloss, LV_OBJ_FLAG_HIDDEN);
        digitalWrite(PIN_LEDPWR, HIGH);
        powerfailtime = millis();
        powerloss = true;
    }
    else if (powerloss && d)
    {
        lv_obj_add_flag(guider_ui.screen_powerloss, LV_OBJ_FLAG_HIDDEN);
        digitalWrite(PIN_LEDPWR, LOW);
        analogWrite(PIN_TFT_BL, 255);
        powerloss = false;
    }
    if (powerloss)
    {
        uint32_t dt = millis() - powerfailtime;
        if (dt > 5000)
        {
            analogWrite(PIN_TFT_BL, 0);
            digitalWrite(PIN_LEDPWR, HIGH);

            SaveConfig();

            SD.end();

            // TODO: Execution engine - Save current state here

            esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BROWNOUT, 1);
            esp_deep_sleep_start();
            // 进入深度睡眠后不会执行后续代码
        }
        if (millis() - dimmtime > 50)
        {
            dimmtime = millis();
            analogWrite(PIN_TFT_BL, 255 - ((millis() - powerfailtime) * 255 / 5000));
        }
    }
    lv_timer_handler(); /* let the GUI do its work */
}

void lvproglistupdate()
{
    String response = "";
    File root = SD.open("/");
    if (root)
    {
        File file = root.openNextFile();
        bool first = true;

        while (file)
        {
            String fname = file.name();
            if (!file.isDirectory() && fname.endsWith(".tbp"))
            {
                if (!first)
                {
                    response += "\n";
                }
                response += fname.substring(0, fname.length() - 4);
                first = false;
            }
            file = root.openNextFile();
        }
        root.close();
    }
    lv_roller_set_options(guider_ui.screen_fileselect, response.c_str(), LV_ROLLER_MODE_INFINITE);
}

void TryRunSelected()
{
    log_i("Try run selected");
    char selbuf[64];
    lv_roller_get_selected_str(guider_ui.screen_fileselect, selbuf, sizeof(selbuf));
    strcpy(selbuf + 1, strdup(selbuf)); // 绝对路径
    selbuf[0] = '/';
    strcpy(selbuf + strlen(selbuf), ".tbp"); // 增加后缀
    log_i("Target:%s", selbuf);
    if (SD.exists(selbuf))
    {
        RunFileAsync(selbuf);
    }
    else
        log_i("Non exists");
}

void RunStateSwitch()
{
    if (config.Running)
    {
        Reset();
        TryRunSelected();
        lv_obj_add_state(guider_ui.screen_fileselect, LV_STATE_DISABLED);
    }
    else
    {
        Reset();
        lv_obj_remove_state(guider_ui.screen_fileselect, LV_STATE_DISABLED);
        // lvproglistupdate();
    }
}