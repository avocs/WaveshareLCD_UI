#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

#include <lvgl.h>
#include "lvgl_port_v8.h"
#include <demos/lv_demos.h>
#include <examples/lv_examples.h>

#include <ui.h> 
#include <actions.h>

// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5     // USB select pin

/*
 ------ STATICS ------- 
*/
static lv_obj_t *test_label = NULL;
static lv_obj_t *mqtt_label = NULL;
static lv_obj_t *time_label = NULL;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static void create_widgets(); 



// EVENT HANDLERS  mmmmm 
void action_close_win(lv_event_t * e) {
    lv_obj_t *win = (lv_obj_t *)lv_event_get_user_data(e);
    // lv_obj_del(win);
    lv_obj_add_flag(win, LV_OBJ_FLAG_HIDDEN);   // hide window again
}


void action_btn1_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {
    Serial.println("Button 1 clicked");
    lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(STOP)), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(objects.statuslabel, get_label_string(STOP)); 

  } 

}
void action_btn2_handler(lv_event_t * e){
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {
    Serial.println("Button 2 clicked");
    lv_obj_set_style_bg_color(objects.toppanel, lv_color_hex(get_color_hex(RUN)), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(objects.statuslabel, get_label_string(RUN)); 
  } 
}
void action_btndd_handler(lv_event_t * e) {
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {
    Serial.println("Button 3 clicked");
    lv_obj_clear_flag(objects.dd_window, LV_OBJ_FLAG_HIDDEN);
  } 
}
void action_btnrm_handler(lv_event_t * e) { 
  lv_event_code_t event_code = lv_event_get_code(e); 
  lv_obj_t *target = lv_event_get_target(e); 
  if (event_code == LV_EVENT_CLICKED || event_code == LV_EVENT_PRESSED) {
    Serial.println("Button 4 clicked");
    lv_obj_clear_flag(objects.remark_window, LV_OBJ_FLAG_HIDDEN);
  } 
}

void action_ta_handler(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    // lv_obj_t * kb = (lv_obj_t*) lv_event_get_user_data(e);

    if (objects.kb == NULL) {
        Serial.println("ERROR: Keyboard pointer is NULL!");
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



void lv_widgets_test(void) {

  font_large = LV_FONT_DEFAULT;
  font_normal = LV_FONT_DEFAULT;

  #if LV_FONT_MONTSERRAT_16
    font_normal = &lv_font_montserrat_16;   // can find these under lvgl/src/font
  #endif 
  // no idea what this should do
  #if LV_USE_THEME_DEFAULT
      lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK,
                            font_normal);
  #endif


}

static void create_widgets() { 


    // logo image on active screen
    LV_IMG_DECLARE(tanand_logo);
    lv_obj_t *logo_bg = lv_img_create(lv_scr_act());
    lv_img_set_src(logo_bg, &tanand_logo);
    lv_obj_align(logo_bg, LV_ALIGN_TOP_MID, 0, 0);

    // style the active screen
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xc233ff), LV_PART_MAIN);  // oh my god 

    // make a tile on active screen
    lv_obj_t *tile1 = lv_obj_create(lv_scr_act()); 
    lv_obj_set_height(tile1, LV_SIZE_CONTENT);        // LV_SIZE_CONTENT auto fits to the content's height 

    // text on tile
    lv_obj_t *someLabel = lv_label_create(tile1);     // define a label on the tile 
    lv_label_set_text(someLabel, "heheheha");         // label text 
    // lv_obj_add_style( ) // i dont get this thing hm 

    // button on tile
    lv_obj_t *someButton = lv_btn_create(tile1); 
    lv_obj_set_height(someButton, LV_SIZE_CONTENT);
    // lv_obj_add_state(someButton, LV_STATE_DISABLED);   // uncomment if button is OFF by default 

    // text on button
    lv_obj_t* btnLabel = lv_label_create(someButton); 
    lv_label_set_text(btnLabel, "WEEEE");
    lv_obj_center(btnLabel);                              // align object to center of parent

    // keyboard pops up on active screen
    lv_obj_t* kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);          // keyboard hidden from sight by default 

    // second tile on active screen 
    lv_obj_t *tile2 = lv_obj_create(lv_scr_act()); 
    lv_obj_set_height(tile2, LV_SIZE_CONTENT);        // LV_SIZE_CONTENT auto fits to the content's height 

    // tile title 
    lv_obj_t* tile2Title = lv_label_create(tile2); 
    lv_label_set_text(tile2Title, " hahahahe"); 
    // i can add styles later ffs 

    // label for text area on tile 2 
    lv_obj_t* crnumLabel = lv_label_create(tile2);
    lv_label_set_text(crnumLabel, "credit card number >:D");         // label text 
    
    lv_obj_t* crnum = lv_textarea_create(tile2); 
    lv_textarea_set_one_line(crnum, true);                // true to configure to one line, false if default
    // lv_textarea_set_password_mode(crnum, true);           // true to enable stars for typing
    lv_textarea_set_placeholder_text(crnum, ":))))))");   // eeeee
    // call back here ! 

    // i am asking a simple question 
    lv_obj_t* oneplusone = lv_label_create(tile2);
    lv_label_set_text(oneplusone, "whats 1+1?");         // label text 

    lv_obj_t* simple_question = lv_dropdown_create(tile2); 
    lv_dropdown_set_options_static(simple_question, "3\nBanananana\nSouth Africa");

    // // slider
    // lv_obj_t* slider1 = lv_slider_create(tile2); 
    // lv_obj_set_width(slider1, LV_PCT(90));            // set the width of the slider to 90% of the tile
    // // lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_ALL, NULL); // LV_EVENT_ALL means to receive all events, 
    //                                                   // custom data set to NULL

    // third tile on active screen 
    lv_obj_t *tile3 = lv_obj_create(lv_scr_act()); 
    lv_obj_set_height(tile3, LV_SIZE_CONTENT);        // LV_SIZE_CONTENT auto fits to the content's height 

    // tile title 
    lv_obj_t* tile3Title = lv_label_create(tile3); 
    lv_label_set_text(tile3Title, "actual stuff now"); 

    // ttatic labels on third tile 
    test_label = lv_label_create(tile3);     // define a label on the tile 
    
    mqtt_label = lv_label_create(tile3);     // define a label on the tile 
    lv_label_set_text(mqtt_label, "0");                // placeholder

    time_label = lv_label_create(tile3); 
    lv_label_set_text(time_label, "Waiting for NTP...");                // placeholder

    // lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x49ff33), LV_PART_MAIN);  // shrek green 

    // test_label = lv_label_create(lv_scr_act());
    // mqtt_label = lv_label_create(lv_scr_act());
    // //initialize the placeholder
    // lv_label_set_text(mqtt_label, "0");
    // lv_obj_align(mqtt_label, LV_ALIGN_BOTTOM_MID, 0, 0);

    // time_label = lv_label_create(lv_scr_act());
    // lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, 0, 0);
    // lv_label_set_text(time_label, "Waiting for NTP...");

// --------------------------------------------------------
    // organise the widgets 
    // set the first tile span across the display
    /*
    grids allow arranging of items into 2D 'table' with rows or cols (tracks)
    track size can be set in pixel, to largest item (LV_GRID_CONTENT) or in Free Unit (FR) 
    to distribute free space proportionally

    functions found under src/extra/layouts/grid/lv_grid.h
    the last parameter must be LV_GRID_TEMPLATE_LAST
    */
    
    // active screen: two main cols equally sized (1:1 ratio), two rows sized to largest item
    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    static lv_coord_t grid_1_col_dsc[] = {LV_GRID_CONTENT, 5, LV_GRID_CONTENT, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_1_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, 10, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*Title*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        30,               /*Boxes*/
        5,                /*Separator*/
        LV_GRID_CONTENT,  /*Box title*/
        30,               /*Boxes*/
        LV_GRID_TEMPLATE_LAST
    };

    // split the screen into cols and rows as described
    lv_obj_set_grid_dsc_array(lv_scr_act(), grid_main_col_dsc, grid_main_row_dsc);

    /* set tile1 to span the entire horizontal screen, starting from col 0, spanning two cols 
                 to span the centre of the vertical screen, starting from row 0, spanning 1 row
    */
    lv_obj_set_grid_cell(tile1, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    /*
    tile1 itself to be split into a grid array to align widgets 
    */
    lv_obj_set_grid_dsc_array(tile1, grid_1_col_dsc, grid_1_row_dsc);
    lv_obj_set_grid_cell(someLabel, LV_GRID_ALIGN_START, 2, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(someButton, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    lv_obj_set_grid_cell(tile2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_dsc_array(tile2, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(tile2Title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(crnum, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(crnumLabel, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    // lv_obj_set_grid_cell(birthdate, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    // lv_obj_set_grid_cell(birthday_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(oneplusone, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 5, 1);
    lv_obj_set_grid_cell(simple_question, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 6, 1);

    lv_obj_set_grid_cell(tile3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_dsc_array(tile3, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(tile3Title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    // lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);
    // lv_obj_set_grid_cell(experience_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(test_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(mqtt_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    // lv_obj_set_grid_cell(team_player_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 5, 1);

}

void setup()
{
    String title = "LVGL porting example";

    Serial.begin(115200);

    pinMode(GPIO_INPUT_IO_4, OUTPUT);
    /**
     * These development boards require the use of an IO expander to configure the screen,
     * so it needs to be initialized in advance and registered with the panel for use.
     */
    Serial.println("Initialize IO expander");
    
    /* Initialize IO expander */
    ESP_IOExpander_CH422G *expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    expander->init();
    expander->begin();

    Serial.println("Set the IO0-7 pin to output mode.");
    // expander->enableAllIO_Output();
    expander->digitalWrite(TP_RST , HIGH);
    expander->digitalWrite(LCD_RST , HIGH);
    expander->digitalWrite(LCD_BL , HIGH);
    delay(100);
    
    // GT911 initialization, must be added, otherwise the touch screen will not be recognized  
    // Initialization begin
    expander->digitalWrite(TP_RST , LOW);
    delay(100);
    digitalWrite(GPIO_INPUT_IO_4, LOW);
    delay(100);
    expander->digitalWrite(TP_RST , HIGH);
    delay(200);
    // Initialization end

    Serial.println(title + " start");

    Serial.println("Initialize panel device");
    ESP_Panel *panel = new ESP_Panel();
    panel->init();
    
#if LVGL_PORT_AVOID_TEAR
    // When avoid tearing function is enabled, configure the RGB bus according to the LVGL configuration
    ESP_PanelBus_RGB *rgb_bus = static_cast<ESP_PanelBus_RGB *>(panel->getLcd()->getBus());
    rgb_bus->configRgbFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
    rgb_bus->configRgbBounceBufferSize(LVGL_PORT_RGB_BOUNCE_BUFFER_SIZE);
#endif

    panel->begin();

    Serial.println("Initialize LVGL");
    lvgl_port_init(panel->getLcd(), panel->getTouch());

    Serial.println("Create UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

// -------------------------- UI CALLED HERE ---------- 

    ui_init();

    // =====================================
    /* Release the mutex */
    lvgl_port_unlock();

    Serial.println(title + " end");
}

static uint8_t cnt = 0;

void loop() {
    char buf[10];  
    sprintf(buf, "%u", cnt);  // Convert cnt to string
    Serial.println("Updating");
    lv_label_set_text(objects.count, buf);

    cnt++;  
    delay(1000);
}
