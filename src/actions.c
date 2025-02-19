// actions.c
// implementation file for event handler functions, and probably a one stop station for modifying objects 

#include <actions.h>
#include <lvgl.h> 
#include <screens.h>

bool submitted = false; 

// EVENT HANDLERS 
void action_close_win(lv_event_t *e) {
    lv_obj_t *win = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_add_flag(win, LV_OBJ_FLAG_HIDDEN);   // hide window again
    lv_obj_add_flag(objects.cfm_label, LV_OBJ_FLAG_HIDDEN);   // hide confirmation label

    if (win == objects.remark_window || win == objects.dd_window) {
        lv_obj_clear_state(objects.btn1, LV_STATE_DISABLED);
        lv_obj_clear_state(objects.btn2, LV_STATE_DISABLED);
        lv_obj_clear_state(objects.btnext, LV_STATE_DISABLED);
    }

    // one of the buttons to be disabled
    if (objects.trigbtn != NULL && submitted) {
        lv_obj_add_state(objects.trigbtn, LV_STATE_DISABLED);
        submitted = false;
    }

    
    // // Enable and disable buttons, based on if a confirmation has been submitted. 
    // if (objects.trigbtn == objects.btn1) { // STOP button
    //     if (submitted) {
    //       // lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(STOP)), LV_PART_MAIN | LV_STATE_DEFAULT);
    //       // lv_label_set_text(objects.statuslabel, get_label_string(STOP));
    //       lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);  // already in stop, disable this button 
    //       submitted = false;    // reset to false
    //     } else {
    //       lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);  // still in resume, disable the resume button  
    //     }
    // }
    // else if (objects.trigbtn == objects.btn2) { // RESUME button
    //     if (submitted) {
    //       // lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(RUN)), LV_PART_MAIN | LV_STATE_DEFAULT);
    //       // lv_label_set_text(objects.statuslabel, get_label_string(RUN));
    //       lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);  // already in resume, disable this button
    //       submitted = false;
    //     } else {
    //       lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);  // still in stop, disable the stop button  
    //     }
    // }
  
    objects.trigbtn = NULL; // Reset after use
}


// pop up the options window 
void action_btndd_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  objects.trigbtn = target;                       // retain the button ptr that triggered the window
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {

    lv_obj_clear_flag(objects.dd_window, LV_OBJ_FLAG_HIDDEN);
    // Disable other buttons
    lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);
    lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);
    lv_obj_add_state(objects.btnext, LV_STATE_DISABLED);
  } 
}




void action_subbtn_handler(lv_event_t * e){

  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  // lv_obj_t *statetrig = (lv_obj_t *)lv_event_get_user_data(e);
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {

    lv_obj_clear_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);      // pop confirmation window
    lv_obj_add_state(objects.subbtn, LV_STATE_DISABLED);        // no more submissions for now 

  } 
}

void action_cfmbox_handler(lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e); 
    lv_obj_t *target = lv_event_get_current_target(e); 

    const char* btn_text = lv_msgbox_get_active_btn_text(target); 

    // if confirm button is pressed, change the status, close msg box
    if (strcmp(btn_text, "Confirm") == 0) {

        submitted = true; 
        if (objects.trigbtn == objects.btn1) {
            lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(STOP)), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(objects.statuslabel, get_label_string(STOP));
            // lv_obj_add_state(objects.btn1, LV_STATE_DISABLED);  // already in stop, disable this button 
            // objects.trigbtn = objects.btn1;                     // save this other button to be enabled 
        } else if (objects.trigbtn == objects.btn2) { // RESUME button
            lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(RUN)), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(objects.statuslabel, get_label_string(RUN));
            // lv_obj_add_state(objects.btn2, LV_STATE_DISABLED);  // already in resume, disable this button
            // objects.trigbtn = objects.btn2;                     // save this other button to be enabled 
        }

        lv_obj_add_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);          // close pop up
        // submitted = false; // reset to false

        
    // if cancel, do nothing and close message box 
    } else if (strcmp(btn_text, "Cancel") == 0) {
        submitted = false;
        // lv_obj_add_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);          // close pop up
        // lv_obj_clear_state(objects.subbtn, LV_STATE_DISABLED);        // reenable submissions
    }
    
    // close pop up either way
    lv_obj_add_flag(objects.cfmbox, LV_OBJ_FLAG_HIDDEN);          // close pop up
    lv_obj_clear_state(objects.subbtn, LV_STATE_DISABLED);        // reenable submissions

}

// void action_btncfm_handler(lv_event_t * e) {
//   // lv_event_code_t event_code = lv_event_get_code(e); 
//   // lv_obj_t *target = lv_event_get_target(e); 
//   // if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {
//   //   lv_obj_clear_flag(objects.cfm_label, LV_OBJ_FLAG_HIDDEN);

//   //   // enable the submit button
//   //   lv_obj_clear_state(objects.subbtn, LV_STATE_DISABLED); 
    
//   // } 
// }


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


void action_dd_handler(lv_event_t * e){

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * dropdown = lv_event_get_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_add_flag(objects.cfm_label, LV_OBJ_FLAG_HIDDEN);

        char buf[30]; 
        lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));
        // Serial.println(buf);

        if(strcmp(buf, "Special Option") == 0) {
            lv_obj_clear_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_clear_flag(objects.dd2_label, LV_OBJ_FLAG_HIDDEN); 
            // lv_obj_add_flag(objects.dd2, LV_OBJ_FLAG_CLICKABLE);
            // lv_obj_clear_state(objects.dd2, LV_STATE_DISABLED);

        } else {
            lv_obj_add_flag(objects.dd2, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_add_flag(objects.dd2_label, LV_OBJ_FLAG_HIDDEN); 
            // lv_obj_clear_flag(objects.dd2, LV_OBJ_FLAG_CLICKABLE);
            // lv_obj_add_state(objects.dd2, LV_STATE_DISABLED);
        }

    }

}