/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_tabview_1;
	lv_obj_t *screen_tabview_1_tab_1;
	lv_obj_t *screen_tabview_1_tab_2;
	lv_obj_t *screen_tabview_1_tab_3;
	lv_obj_t *screen_tabview_1_tab_4;
	lv_obj_t *screen_fileselect;
	lv_obj_t *screen_label_12;
	lv_obj_t *screen_run;
	lv_obj_t *screen_DI1;
	lv_obj_t *screen_AI1;
	lv_obj_t *screen_label_1;
	lv_obj_t *screen_DO4;
	lv_obj_t *screen_DI4;
	lv_obj_t *screen_DI3;
	lv_obj_t *screen_DI2;
	lv_obj_t *screen_label_2;
	lv_obj_t *screen_DO1;
	lv_obj_t *screen_DO2;
	lv_obj_t *screen_DO3;
	lv_obj_t *screen_label_3;
	lv_obj_t *screen_AI2;
	lv_obj_t *screen_label_6;
	lv_obj_t *screen_label_7;
	lv_obj_t *screen_TX1;
	lv_obj_t *screen_TX2;
	lv_obj_t *screen_RX1;
	lv_obj_t *screen_RX2;
	lv_obj_t *screen_stath;
	lv_obj_t *screen_stath_label;
	lv_obj_t *screen_label_13;
	lv_obj_t *screen_label_14;
	lv_obj_t *screen_label_15;
	lv_obj_t *screen_nonvolatile;
	lv_obj_t *screen_label_16;
	lv_obj_t *screen_autoboot;
	lv_obj_t *screen_staterecover;
	lv_obj_t *screen_label_17;
	lv_obj_t *screen_label_18;
	lv_obj_t *screen_wireless_connqr;
	lv_obj_t *screen_label_20;
	lv_obj_t *screen_label_19;
	lv_obj_t *screen_wireless;
	lv_obj_t *screen_powerloss;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen(lv_ui *ui);

LV_FONT_DECLARE(lv_font_SourceHanSansCNBold2_15)
LV_FONT_DECLARE(lv_font_SourceHanSansCNBold2_18)
LV_FONT_DECLARE(lv_font_montserratMedium_10)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_montserratMedium_15)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_SourceHanSansCNBold2_10)
LV_FONT_DECLARE(lv_font_SourceHanSansCNBold2_14)
LV_FONT_DECLARE(lv_font_SourceHanSansCNBold2_20)


#ifdef __cplusplus
}
#endif
#endif
