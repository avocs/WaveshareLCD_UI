#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include "globals.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;



void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 480);
    {
        lv_obj_t *parent_obj = obj;
        {
            // toppanel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.toppanel = obj;
            lv_obj_set_pos(obj, 50, 20);
            lv_obj_set_size(obj, 700, 80);
            lv_obj_set_style_bg_color(obj, lv_color_hex(get_color_hex(current_machine_state)), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.stationid = obj;
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "Station ID:");
                    lv_obj_set_style_align(obj, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

                }
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.statuslabel = obj; 
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, get_label_string(IDLE));
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // midpanel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.midpanel = obj;
            lv_obj_set_pos(obj, 50, 120);
            lv_obj_set_size(obj, 700, 210);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    // lv_obj_set_pos(obj, 0, 5);
                    objects.count = obj;
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "NO VALUE");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // botpanel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.botpanel = obj;
            lv_obj_set_pos(obj, 50, 350);
            lv_obj_set_size(obj, 700, 100);
            lv_obj_set_style_layout(obj, LV_LAYOUT_GRID, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            // DEFINE GRID LAYOUT
            {   // four equidistance spaces for buttons 
                static lv_coord_t dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
                lv_obj_set_style_grid_row_dsc_array(obj, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            {
                static lv_coord_t dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
                lv_obj_set_style_grid_column_dsc_array(obj, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
            }

            { 
              // alternatively, 
                // static lv_coord_t botpanel_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
                // static lv_coord_t botpanel_row_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
                // lv_obj_set_grid_dsc_array(obj, botpanel_col_dsc, botpanel_row_dsc);

            }
            
              // BUTTONS FOR THE THINGY
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btn1
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.btn1 = obj;
                    // lv_obj_set_pos(obj, 400, 357);
                    lv_obj_set_size(obj, 130, 50);
                    // lv_obj_add_event_cb(obj, action_btn1_handler, LV_EVENT_PRESSED, (void *)0);
                    lv_obj_add_event_cb(obj, action_btndd_handler, LV_EVENT_PRESSED, (void *)0);

                    // SPECIFY GRID ALIGNMENT 
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0,1, LV_GRID_ALIGN_CENTER, 0,1);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, get_label_string(STOP));
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // btn2
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.btn2 = obj;
                    // lv_obj_set_pos(obj, 437, 381);
                    lv_obj_set_size(obj, 130, 50);
                    // lv_obj_add_event_cb(obj, action_btn2_handler, LV_EVENT_PRESSED, (void *)0);
                    lv_obj_add_event_cb(obj, action_btndd_handler, LV_EVENT_PRESSED, (void *)0);
                    // SPECIFY GRID ALIGNMENT 
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1,1, LV_GRID_ALIGN_CENTER, 0,1);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, get_label_string(RESUME));
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // btnext
                    lv_obj_t *obj = lv_btn_create(parent_obj);
                    objects.btnext = obj;
                    lv_obj_set_size(obj, 130, 50);
                    // lv_obj_add_event_cb(obj, action_btnrm_handler, LV_EVENT_PRESSED, (void *)0);

                    // SPECIFY GRID ALIGNMENT 
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2,1, LV_GRID_ALIGN_CENTER, 0,1);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, get_label_string(EXTRA));
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }

        {
            // dd_window
            lv_obj_t *obj = lv_win_create(parent_obj, 50);
            objects.dd_window = obj;
            lv_obj_set_pos(obj, 120, 25);
            lv_obj_set_size(obj, 540, 280);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);


            // ADD A CLOSE BUTTON
            lv_obj_t * closebtn = lv_win_add_btn(obj, LV_SYMBOL_CLOSE, 50);
            objects.closebtn = closebtn;
            lv_obj_add_event_cb(closebtn, action_close_win, LV_EVENT_CLICKED, obj);

            lv_win_add_title(obj, " Title");
            lv_obj_t * subbtn = lv_win_add_btn(obj, LV_SYMBOL_OK, 50);
            objects.subbtn = subbtn; 
            lv_obj_add_state(subbtn, LV_STATE_DISABLED);
            lv_obj_add_event_cb(subbtn, action_subbtn_handler, LV_EVENT_CLICKED, obj);

            {
                lv_obj_t *parent_obj = obj;
                // OBTAIN INTERNAL CONTENT 
                lv_obj_t *cont = lv_win_get_content(parent_obj);
                // trying to see if this works 
                lv_obj_set_style_layout(cont, LV_LAYOUT_GRID, LV_PART_MAIN | LV_STATE_DEFAULT);

                { 
                    static lv_coord_t window_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
                    static lv_coord_t window_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_grid_dsc_array(cont, window_col_dsc, window_row_dsc);

                }
                {
                    // dd_label
                    lv_obj_t *obj = lv_label_create(cont);
                    objects.dd_label = obj;
                    // lv_obj_set_pos(obj, 0, 0);
                    // lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    // lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 5);
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, 0,1, LV_GRID_ALIGN_CENTER, 0,1);
                    lv_label_set_text(obj, "Reason:");

                }
                {
                    // dd1
                    lv_obj_t *obj = lv_dropdown_create(cont);
                    objects.dd1 = obj;
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0,1, LV_GRID_ALIGN_CENTER, 1,1);
                    lv_dropdown_set_options(obj, get_label_string(DD1OPTIONS));
                    lv_obj_add_event_cb(obj, action_dd_handler, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, action_ddupdate_handler, LV_EVENT_READY, (void *)0);
                    update_current_reason(current_down_reason, lv_dropdown_get_options(obj));
                    // current_special_indices = (int[]) {DEMO_SPECIAL_OPTION_INDEX}; 
                    current_special_indices_count = sizeof(current_special_indices)/sizeof(int);
  
                }
                {
                    // dd2_label
                    lv_obj_t *obj = lv_label_create(cont);
                    objects.dd2_label = obj;
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); 
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, 1,1, LV_GRID_ALIGN_CENTER, 0,1);
                    lv_label_set_text(obj, "Station:");
                }
                {
                    // dd2
                    lv_obj_t *obj = lv_dropdown_create(cont);
                    objects.dd2 = obj;
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); 
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1,1, LV_GRID_ALIGN_CENTER, 1,1);
                    lv_dropdown_set_options(obj, get_label_string(DD2OPTIONS));
                    lv_obj_add_event_cb(obj, action_dd_handler, LV_EVENT_VALUE_CHANGED, (void *)0);
                    lv_obj_add_event_cb(obj, action_ddupdate_handler, LV_EVENT_READY, (void *)0);
                    update_current_reason(current_station_reason, lv_dropdown_get_options(obj));
                  
                }
                {
                    // rm_label
                    lv_obj_t *obj = lv_label_create(cont);
                    objects.rm_label_dd = obj;
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, 0,1, LV_GRID_ALIGN_CENTER, 2,1);
                    lv_label_set_text(obj, "Remark:");
                }
                {
                    // rm_ta_dd
                    lv_obj_t *obj = lv_textarea_create(cont);
                    objects.rm_ta_dd = obj;
                    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0,1, LV_GRID_ALIGN_CENTER, 3,1);
                    lv_textarea_set_max_length(obj, 128);
                    lv_textarea_set_one_line(obj, false);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, action_ta_handler, LV_EVENT_FOCUSED, (void *)0);
                    lv_obj_add_event_cb(obj, action_ta_handler, LV_EVENT_DEFOCUSED, (void *)0);
                    lv_textarea_set_placeholder_text(obj, "Enter text here...");
                  
                }
            }
        }
        {
            // cfm_box 
            static const char* btns[] = {"Confirm", "Cancel", ""};
            lv_obj_t *obj = lv_msgbox_create(parent_obj, "Confirmation", "Proceed?", btns, false); 
            objects.cfmbox = obj; 
            lv_obj_set_pos(obj, 400, 100);
            lv_obj_set_size(obj, 240, 150);
            lv_obj_add_event_cb(obj, action_cfmbox_handler, LV_EVENT_VALUE_CHANGED, NULL); 
            // lv_obj_add_event_cb(lv_msgbox_get_btns(obj), action_cfmbox_handler, LV_EVENT_VALUE_CHANGED, NULL); 
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); 

        }

        {
            // kb
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.kb = obj;
            lv_obj_set_pos(obj, 0, 250);
            lv_obj_set_size(obj, 800, 210);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, action_kb_handler, LV_EVENT_READY, (void *)0);
            lv_obj_add_event_cb(obj, action_kb_handler, LV_EVENT_CANCEL, (void *)0);
        }

    }

    // BIND KEYBOARD TO THE TEXT AREA
    lv_keyboard_set_textarea(objects.kb, objects.rm_ta_dd);
    
    tick_screen_main();
}

void tick_screen_main() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
