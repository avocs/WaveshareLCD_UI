#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif


extern void action_btndd_handler(lv_event_t * e);
extern void action_ta_handler(lv_event_t * e);
extern void action_kb_handler(lv_event_t * e);
extern void action_dd_handler(lv_event_t * e);

// added manually
extern void action_close_win(lv_event_t * e);
extern void action_subbtn_handler(lv_event_t * e);
extern void action_cfmbox_handler(lv_event_t * e);
extern void action_ddupdate_handler(lv_event_t * e);




#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/