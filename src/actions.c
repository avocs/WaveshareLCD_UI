// actions.c
// implementation file for event handler functions, and probably a one stop station for modifying objects 

#include <actions.h>
#include <lvgl.h> 
#include <screens.h>
#include <globals.h>
#include "mqtt_publisher.h"
#include <Arduino.h> // for String lmao




// EVENT HANDLERS 
void action_close_win(lv_event_t *e) {
    lv_obj_t *win = (lv_obj_t *)lv_event_get_user_data(e);

    // RESET PANEL 
    lv_obj_add_flag(win, LV_OBJ_FLAG_HIDDEN);               // hide window
    lv_obj_clear_state(objects.btn1, LV_STATE_DISABLED);
    lv_obj_clear_state(objects.btn2, LV_STATE_DISABLED);
    lv_obj_clear_state(objects.btnext, LV_STATE_DISABLED);

    /*
    conditions of publishing: 1. for stops and resumes without autostops, submission approved
                              2. resumes flag reason as autostops?
    */

    // if flag to publish approved, 
    if (submitted && objects.trigbtn != NULL) {
        lv_obj_add_state(objects.trigbtn, LV_STATE_DISABLED);

        // STORE REASON AND REMARKS 
        char buf[128]; 

        lv_dropdown_get_selected_str(objects.dd1, buf, sizeof(buf));
        strcpy(reason, buf);

        // if a specific reason is given,
        if (!lv_obj_has_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN)) {
            lv_dropdown_get_selected_str(objects.dd2, buf, sizeof(buf));
            strcpy(specReason, buf);
        } else {
            strcpy(specReason, "n/a");  // default to n/a
        }
        // Copy remarks from textarea
        strcpy(remark, lv_textarea_get_text(objects.rm_ta_dd));

        char *json_str = generate_report_json(reason, specReason, remark);
        if (json_str != NULL) {
            if (objects.trigbtn == objects.btn1) {
              queue_mqtt_publish(stop_topic, json_str, 0, false);
            } else if (objects.trigbtn == objects.btn2) {
              queue_mqtt_publish(resume_topic, json_str, 0, false);
            }
            free(json_str); 
        } else {
            ESP_LOGE(TAG, "Failed to create JSON string");
        }

        submitted = false;      // reset to false
      
      // special condition: no auto_stop flag detected on resume
    } else if ((objects.trigbtn == objects.btn2) && !auto_stop_flag) {
        char* json_str = generate_report_json("No autostop", "n/a", "");
        queue_mqtt_publish(resume_topic, json_str, 0, false);
        free(json_str);
        lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);        // disable resume button
    
    // not submitted by default
    } else if (objects.trigbtn != NULL) {
        button_check(objects.btn1, objects.btn2, current_machine_state);

        // if (objects.trigbtn == objects.btn1) {    // if the stop hasnt been approved
        //     lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);    // the resume button remains disabled
        // } else if (objects.trigbtn == objects.btn2) {     // if the resume hasnt been submitted
        //     lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);    // the stop button remains disabled 
        // }
    }


    objects.trigbtn = NULL; // Reset after use

    lv_dropdown_set_selected(objects.dd1, 0);
    lv_dropdown_set_selected(objects.dd2, 0);
    lv_textarea_set_text(objects.rm_ta_dd, "");              // blank out the option
    lv_obj_add_state(objects.subbtn, LV_STATE_DISABLED);
    lv_obj_add_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN);       // hide specifics dropdown

}


// pop up the options window 
void action_btndd_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  objects.trigbtn = target;                       // retain the button ptr that triggered the window

  // pop up the reason window if (autostop is on, or the stop button is pressed 
  if (auto_stop_flag || target == objects.btn1) { 

    // display the reason window
    lv_obj_clear_flag(objects.dd_window, LV_OBJ_FLAG_HIDDEN);
    // disable other buttons 
    lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);
    lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);
    lv_obj_add_state(objects.btnext, LV_STATE_DISABLED);

  } else {  // assuming this is the "no condition resume", set to resume directly
      set_top_panel_status(objects.toppanel, objects.statuslabel, RUN);
      current_machine_state = RUN; 
      lv_event_send(objects.closebtn, LV_EVENT_CLICKED, objects.dd_window);
  }
}



// submit button
void action_subbtn_handler(lv_event_t * e){

  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  // lv_obj_t *statetrig = (lv_obj_t *)lv_event_get_user_data(e);
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {

    lv_obj_clear_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);      // pop confirmation window
    lv_obj_add_state(objects.subbtn, LV_STATE_DISABLED);        // no more submissions for now 

  } 
}

// confirmation box 
void action_cfmbox_handler(lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e); 
    lv_obj_t *target = lv_event_get_current_target(e); 

    const char* btn_text = lv_msgbox_get_active_btn_text(target); 

    // if confirm button is pressed, change the status, close msg box
    if (strcmp(btn_text, "Confirm") == 0) {

        submitted = true; 
        if (objects.trigbtn == objects.btn1) {
            set_top_panel_status(objects.toppanel, objects.statuslabel, STOP);
            current_machine_state = STOP; 
            // lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);  // already in stop, disable this button 
            // objects.trigbtn = objects.btn1;                     // save this other button to be enabled 
        } else if (objects.trigbtn == objects.btn2) { // RESUME button
            set_top_panel_status(objects.toppanel, objects.statuslabel, RUN);
            current_machine_state = RUN; 
            // lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);  // already in resume, disable this button
            // objects.trigbtn = objects.btn2;                     // save this other button to be enabled 
        }

        lv_event_send(objects.closebtn, LV_EVENT_CLICKED, objects.dd_window); // send event to close window
        
        
    // if cancel, do nothing
    } else if (strcmp(btn_text, "Cancel") == 0) {
        submitted = false;
        lv_event_send(objects.dd1, LV_EVENT_VALUE_CHANGED, NULL);           // process the dropdown again to determine state of confirm button
    }
    
    lv_obj_add_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);          // close pop up


}

void action_ta_handler(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if (objects.kb == NULL) {
        return;
    }

    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(objects.kb, ta);
        lv_obj_clear_flag(objects.kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_t * active_ta = lv_keyboard_get_textarea(objects.kb);
        if(active_ta == ta) {
            lv_keyboard_set_textarea(objects.kb, NULL);
            lv_obj_add_flag(objects.kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void action_kb_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);

    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_t * ta = lv_keyboard_get_textarea(kb);
        if(ta) {
          lv_event_send(ta, LV_EVENT_DEFOCUSED, NULL);    // send defocus event
          lv_indev_reset(NULL, ta);       // needed this reset to allow reclicking of ta after cancelling 
          // src: https://forum.lvgl.io/t/keyboard-lv-event-ready-on-press-ok-button-does-not-defocus-textarea/10855/2 
        }
    }



}

// dropdown option handler
void action_dd_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * dropdown = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {

        char buf[30]; 
        uint8_t optind; 
        lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
        optind = lv_dropdown_get_selected(dropdown);

        if (strcmp(buf, "-") != 0) {
              lv_obj_clear_state(objects.subbtn, LV_STATE_DISABLED);  // enable the submit button for the reason given
          // filter special options only for the first dropdown
          if (dropdown == objects.dd1) {
              // hidden by default
              lv_obj_add_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN); 
              lv_obj_add_flag(objects.dd2_label, LV_OBJ_FLAG_HIDDEN); 
              lv_dropdown_set_selected(objects.dd2, 0);               // set to '-'
              // im assuming the dropdowns have been updated upon call to READY, so the object index will find itself in the selection list
              for (int i = 0; i < current_special_indices_count; i++) {
                if (optind == current_special_indices[i]) {
                // if (optind == DEMO_SPECIAL_OPTION_INDEX) {
                  lv_obj_add_state(objects.subbtn, LV_STATE_DISABLED);  // disable the submit button since it will always fall to '-'  
                  lv_obj_clear_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN); 
                  lv_obj_clear_flag(objects.dd2_label, LV_OBJ_FLAG_HIDDEN); 
                  break;
                }
              }
          }
        } else { 
            lv_obj_add_state(objects.subbtn, LV_STATE_DISABLED);  // disable the submit button if no reason given
        }
    }
}


// update dropdown list 
void action_ddupdate_handler(lv_event_t * e) {

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * dropdown = lv_event_get_target(e);

    if (dropdown == objects.dd1 && ddd_update_required) {
      update_current_reason(current_down_reason, latest_down_reason);
      memcpy(current_special_indices, latest_special_indices, sizeof(int) * latest_special_indices_count);
      current_special_indices_count = latest_special_indices_count;
      lv_dropdown_set_options_static(dropdown, current_down_reason); 
      ddd_update_required = false; 
    } else if (dropdown == objects.dd2 && sdd_update_required) {
      update_current_reason(current_station_reason, latest_station_reason);
      lv_dropdown_set_options_static(dropdown, current_station_reason);
    }

}

