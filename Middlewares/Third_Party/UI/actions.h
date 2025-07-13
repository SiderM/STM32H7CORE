#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_set_backlight(lv_event_t * e);
extern void action_reset_backlight(lv_event_t * e);
extern void action_change_theme(lv_event_t * e);
extern void action_set_loading(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/