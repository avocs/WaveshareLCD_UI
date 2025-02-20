#ifndef GLOBALS_H
#define GLOBALS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <lvgl.h>

#define MAX_OPTIONS_LENGTH 256                        // MAX LENGTH OF DROP DOWN REASONS AFTER CONCATENATION
#define MAX_SELECTION_LENGTH 128                      // MAX LENGTH OF SINGULAR REASON/REMARK INPUT ALLOWED
#define MAX_OPTIONS_COUNT 10                          // MAX NUMBER OF OPTIONS FOR DROPDOWNS
#define DEMO_SPECIAL_OPTION_INDEX 4                   // MAX NUMBER OF OPTIONS FOR DROPDOWNS
#define MAX_PAYLOAD_LENGTH 256                        // MAX LENGTH OF PAYLOAD FOR PUBLISHING

// Shared state variables
extern int bottle_count;                              // REC: number of bottles
extern bool current_full_state;                               // REC: panel color fill flag
extern bool latest_full_state;                               // REC: panel color fill flag
extern int current_machine_state;                             // PUB: state of machine (RUN/IDLE/SLOW/STOP) 
extern int latest_machine_state;                             // PUB: state of machine (RUN/IDLE/SLOW/STOP) 
extern bool auto_stop_flag;                            // REC: state of auto_stop triggering changes
extern char current_down_reason[MAX_OPTIONS_LENGTH];  // REC: current list of down reasons, to check for dropdown updates
extern char latest_down_reason[MAX_OPTIONS_LENGTH];   // REC: latest list of down reasons, for dropdown updates
extern char current_station_reason[MAX_OPTIONS_LENGTH];  // REC: current list of down reasons, to check for dropdown updates
extern char latest_station_reason[MAX_OPTIONS_LENGTH];   // REC: latest list of down reasons, for dropdown updates
extern int current_special_indices[MAX_OPTIONS_COUNT];   // REC: latest list of down reasons, for dropdown updates
extern int latest_special_indices[MAX_OPTIONS_COUNT];   // REC: latest list of down reasons, for dropdown updates
extern int current_special_indices_count;   // REC: latest list of down reasons, for dropdown updates
extern int latest_special_indices_count;   // REC: latest list of down reasons, for dropdown updates
extern char remark[MAX_SELECTION_LENGTH];             // PUB: keyboard input remark
extern char reason[MAX_SELECTION_LENGTH];             // PUB: reason selected from dropdown box
extern char specReason[MAX_SELECTION_LENGTH];         // PUB: reason selected from dropdown box
extern char ID[MAX_SELECTION_LENGTH];          // PUB: machine identifier string

extern bool submitted;                                // INTERNAL: check for button submission to enable and disable widgets 
extern bool ddd_update_required;                       // INTERNAL: check for updates before opening dropdown box 
extern bool sdd_update_required;                       // INTERNAL: check for updates before opening dropdown box 


// MQTT TOPICS
extern const char *current_line;                      // NAME FOR THE CURRENT LINE
extern const char *stop_topic;                        // TOPIC TO PUBLISH REPORT TO WHEN STOPPED
extern const char *resume_topic;                      // TOPIC TO PUBLISH REPORT TO WHEN RESUMED
extern const char *bottle_count_topic;                // TOPIC TO READ BOTTLE COUNT FROM
extern const char *line_status_topic;             // TOPIC TO CHECK RUN STATUS AND AUTO STOP FROM
extern const char *down_reason_topic;                 // TOPIC TO OBTAIN REASONS FOR MACHINE DOWN FROM
extern const char *station_reason_topic;                 // TOPIC TO OBTAIN REASONS FOR MACHINE DOWN FROM

// ENUM FOR COMMON LABELS 
typedef enum {
    RUN,
    SLOW,
    STOP,
    IDLE,
    FULL,
    NOT_FULL,
    RESUME,
    REMARK,
    EXTRA, 
    DD1OPTIONS,
    DD2OPTIONS,
    LABEL_COUNT           // Special value to keep track of enum count
} labels; 

// STRINGS CORRESPONDING TO LABEL INDEX 
static const char *label_strings[LABEL_COUNT] = {
    "RUN",
    "SLOW",
    "STOP",
    "IDLE",
    "FULL",
    "NOT FULL",
    "RESUME",
    "REMARK",
    "EXTRA",
    // these are defaults, likely overridden soon 
    "-\nOption 1\nOption 2\nOption 3 is a very long option\nSpecial Option\nOption 5\nOption 6",
    "-\nOption 1\nOption 2\nOption 3 is a very long option\nOption 4\nOption 5\nOption 6"
};

// GETTERS
int get_color_hex(labels label);
const char *get_label_string(labels label);
void set_top_panel_status(lv_obj_t * top_panel_obj, lv_obj_t * status_label_obj, labels label);
void button_check(lv_obj_t * btn1, lv_obj_t * btn2, labels label);
// (Optional) Getter/setter prototypes if you want encapsulation
void set_bottle_count(int count);
int get_bottle_count();

void set_line_running(bool running);
bool get_line_running();

void update_current_reason(char* prev_down_reasons, const char *new_down_reasons);
const char *get_current_down_reason();
const char *get_current_station_reason();


#ifdef __cplusplus
}
#endif

#endif // APP_STATE_H