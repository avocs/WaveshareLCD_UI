#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *toppanel;
    lv_obj_t *stationid;
    lv_obj_t *statuslabel;
    lv_obj_t *midpanel;
    lv_obj_t *count;
    lv_obj_t *botpanel;
    lv_obj_t *btn1;
    lv_obj_t *btn2;
    lv_obj_t *btnext;
    // lv_obj_t *remark_window;
    lv_obj_t *rm_label;
    // lv_obj_t *rm_ta;
    lv_obj_t *kb;
    lv_obj_t *dd_window;
    lv_obj_t *dd_label;
    lv_obj_t *dd1;
    lv_obj_t *dd2_label;
    lv_obj_t *dd2;
    lv_obj_t *rm_label_dd;
    lv_obj_t *rm_ta_dd;
    lv_obj_t *btncfm;
    // lv_obj_t *cfm_label;
    lv_obj_t *closebtn;
    lv_obj_t *subbtn;
    lv_obj_t *cfmbox;

    lv_obj_t *trigbtn;    // button check flag 
} objects_t;

extern objects_t objects;






enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/