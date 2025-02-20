#include "globals.h"
#include <string.h>

int bottle_count = 0;
bool current_full_state = false;
bool latest_full_state = false;
char current_down_reason[MAX_OPTIONS_LENGTH] = {0};
char latest_down_reason[MAX_OPTIONS_LENGTH] = {0};
char current_station_reason[MAX_OPTIONS_LENGTH] = {0};
char latest_station_reason[MAX_OPTIONS_LENGTH] = {0};
int current_special_indices[] = {DEMO_SPECIAL_OPTION_INDEX};   // REC: latest list of down reasons, for dropdown updates
int latest_special_indices[MAX_OPTIONS_COUNT];   // REC: latest list of down reasons, for dropdown updates
int current_special_indices_count = 1;   // REC: la
int latest_special_indices_count;   // REC: la
char remark[MAX_SELECTION_LENGTH] = {0};
char reason[MAX_SELECTION_LENGTH] = {0};
char specReason[MAX_SELECTION_LENGTH] = {0};
char ID[MAX_SELECTION_LENGTH] = {0};
int latest_machine_state = IDLE; 
int current_machine_state = IDLE; 
bool auto_stop_flag = 1; 
bool submitted = false;
bool ddd_update_required = false;                       // INTERNAL: check for updates before opening dropdown box 
bool sdd_update_required = false;                       // INTERNAL: check for updates before opening dropdown box 


int get_color_hex(labels label) {
      switch (label) {
        case RUN:       return 0xff5dcf61;    // some shade of green  
        case SLOW:      return 0xffedd51a;    // some shade of yellow
        case STOP:      return 0xffe02d2d;    // some shade of red 
        case IDLE:      return 0xffb4b5a8;    // some shade of grey
        case FULL:      return 0Xfff0a95d;    // some shade of orange
        case NOT_FULL:  return 0xffffffff;    // also white 
        default:        return 0xffffffff;    // white 
      }
}


void set_top_panel_status(lv_obj_t * top_panel_obj, lv_obj_t * status_label_obj, labels label) {
    lv_obj_set_style_bg_color(top_panel_obj, lv_color_hex(get_color_hex(label)), LV_PART_MAIN);
    lv_label_set_text(status_label_obj, get_label_string(label));
}


void button_check(lv_obj_t * btn1, lv_obj_t * btn2, labels label) {
    switch(label){
      case RUN:
        lv_obj_clear_state(btn1, LV_STATE_DISABLED);          // run btn disabled 
        lv_obj_add_state(btn2, LV_STATE_DISABLED);
        break;
      case SLOW:
      case IDLE: 
        lv_obj_clear_state(btn1, LV_STATE_DISABLED);          // both buttons enabled
        lv_obj_clear_state(btn2, LV_STATE_DISABLED);
        break; 
      case STOP: 
        lv_obj_add_state(btn1, LV_STATE_DISABLED);          // stop btn disabled
        lv_obj_clear_state(btn2, LV_STATE_DISABLED);
        break;
      default: 
        lv_obj_clear_state(btn1, LV_STATE_DISABLED);          // both buttons enabled
        lv_obj_clear_state(btn2, LV_STATE_DISABLED);
        break; 
    }
    
}


const char *get_label_string(labels label) {
    if (label < 0 || label >= LABEL_COUNT) {
        return "UNKNOWN";
    }
    return label_strings[label];
}


// Implement optional getters/setters
void set_bottle_count(int count) {
    bottle_count = count;
}

int get_bottle_count() {
    return bottle_count;
}


void update_current_reason(char* prev_down_reasons, const char *new_down_reasons) {
    strncpy(prev_down_reasons, new_down_reasons, MAX_OPTIONS_LENGTH - 1);
    prev_down_reasons[MAX_OPTIONS_LENGTH - 1] = '\0';
}

const char *get_current_down_reason() {
    return current_down_reason;
}
const char *get_current_station_reason() {
    return current_station_reason;
}
