# WaveShare-ESP32-S3-Touch-LCD-7 UI

Development platform: ArduinoIDE
Documentation: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-7 


## A. File structure
### GENERATED FILES
- `ui.c, ui.h`: contains ui_init() function called to generate UI (equivalent to calling lv_demo_widgets())
- `screens.c, screens.h`: generates screens with all the widgets 
- `actions.h`: header file for all event callbacks, EEZ requires you to manually implement the code for callbacks
- `fonts, images, styles, structs`: if fonts, styles, images are imported into EEZ, corresponding src code files will be generated here to be utilized by widgets
- `vars`: contains global variables/flow variables declared within EEZ 

### CONFIG FILES (imported from Waveshare docs)
- 'ESP_Panel_Board_Custom.h, ESP_Panel_Conf.h`: ESP_Panel config files for this specific board, as required (and stated) by the ESP_Display_Panel library
- `lvgl_port_v8`: Configures the display mode, and handles refresh tasks for the screen.

### ADDED FILES
- `LVGL_COUNTER_DISP_UI`: essentially main.c, a derived file which extends the `lvgl_Porting` example given in the docs
- 'actions.c': Implementation file for all the event callbacks as declared in `actions.h`
- `globals`: Container for global variables and functions
- `mqtt_publisher`: Contains helper functions for formatting JSON objects and sending them to a queue for publishing
 
---- 
## B. Dependencies
All libraries imported as provided by Waveshare. 

- LVGL v8.4 -- latest v9.2, but the newer version has many of the functions renamed. could upgrade to this version with function name changes.
	- Docs: https://docs.lvgl.io/8.4/
- ESP32_Display_Panel -- Waveshare offers a derived version which is very far behind from the latest git release)
	- note the git release now has official support for this board
- ESP32_IO_Expander -- also offered but somewhat behind the latest git release, but have not encountered issues with it yet, outside of renaming a few macros
- `lv_conf.h` -- A copy of the blank template provided in `lvgl/lv_conf_template.h`, to define LVGL macros and allow use of certain font files, examples etc. included in the library. 

---- 
## C. Generating a UI project template using EEZ Studio

GitHub installation: https://github.com/eez-open/studio

### Steps: 
1. Follow the GitHub README installation instructions to install EEZ Studio in your device (I used the git clone/npm start method)
2. In EEZ Studio, set up a new project using LVGL 8.x
3. Drag and drop widgets as required, establish the required parent-child relations between widgets
	- edited styles will be reflected in the generated files
	- if callback functions are necessary, add it to the `User Actions` tab, and a function prototype will be given in `actions.h`


### Limitations:

#### Manual Components
Some of the components should be manually added, for example: 

1. Tabs: 
	- the current UI is a singular screen, but if tabs must be added, this has to be created entirely manually outside of EEZ, generated code tends to link all tab buttons to the same current screen, so effectively useless buttons.
	- tabs and screens are different structures, calls to use 'load screen' related functions have to be changed in `ui.c`and `screens.c` to use tabs

2. Layouts:
	- EEZ does not handle widget layouts well, and tends to confuse the parent-object relation for widgets using the grid
	- adding code lines manually to each widget using a grid is way easier

3. Windows: (not the OS) 
	- By default, EEZ windows are empty, with a close button that cannot be removed in the software
	- all widgets (labels, buttons, text areas, etc.) cannot be dragged into the window as a child widget, it all goes directly under the window
	- Could use EEZ to create the empty window, edit size and position, and generate the corresponding template code as a starter. Add all child widgets manually.

4. MsgBox: 
	- If the message box is created and deleted within a callback function, then it has to be created in the callback function implementation, not in `screens.c`
	- The generated code missed out on using LVGL's "add buttons" function directly, which is a way faster method of creating buttons for message boxes. 
	- Like windows, could use EEZ to create the empty msgbox, edit size and position, then generate the corresponding template code as a starter. Add all child widgets or buttons manually.


