#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_btn1_handler(lv_event_t * e);
extern void action_btn2_handler(lv_event_t * e);
extern void action_btndd_handler(lv_event_t * e);
extern void action_btnrm_handler(lv_event_t * e);
extern void action_ta_handler(lv_event_t * e);

// added manually
extern void action_close_win(lv_event_t * e);




#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/