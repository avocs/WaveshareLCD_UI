#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

#include <lvgl.h>
#include "lvgl_port_v8.h"

#include <ui.h> 
#include "globals.h"
#include "mqtt_publisher.h"
#include <cJSON.h>

#include <esp_wifi.h>
#include <esp_event_base.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sntp.h>
#include <time.h>
#include <mqtt_client.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5     // USB select pin

/*
 ------ STATICS ------- 
*/

static const lv_font_t * font_large;
static const lv_font_t * font_normal;
static lv_style_t style_text_muted;
static lv_style_t style_title;

// -----  WIFI SETTINGS -------
// const char *ssid        = "TanandWorkspace"; // Enter your WiFi name
const char *ssid        = "Tanand_Hardware"; // Enter your WiFi name
const char *password    = "202040406060808010102020";  // Enter WiFi password
// -----  MQTT BROKER SETTINGS -------
// const char *mqtt_broker = "mqtt://tanandtech.com.my";
const char *mqtt_broker = "mqtt://192.168.0.101/";
const char *topic       = "random/test/esp32s3lcd-1";
const char *r_topic     = "random/test/send";
const int mqtt_port     = 1883;
static const char* TAG  = "ESP32";

// char buffers 

// Counter
int counter             = 0;
float data              = 0.0;
bool wifi_connected     = false;
esp_mqtt_client_handle_t client;
QueueHandle_t countQueue, statusQueue, downoptionsQueue, stationoptionsQueue;

typedef struct {
  char topic[64];
  char payload[256];
  int qos;
  bool retain; 
} PubMessage; 


// MQTT TOPICS
const char *current_line; 
const char *stop_topic = "thermos/esp/stop"; 
const char *resume_topic = "thermos/esp/resume"; 
const char *bottle_count_topic = "thermos/esp/count"; 
const char *line_status_topic = "thermos/esp/status"; 
const char *down_reason_topic = "thermos/esp/downreason"; // fixed topic 
const char *station_reason_topic = "thermos/esp/stationreason"; // fixed topic 

// Buffers for topic strings
char current_line_buffer[64];
char stop_topic_buffer[64];
char resume_topic_buffer[64];
char bottle_count_topic_buffer[64];
char line_status_topic_buffer[64];


// ================ MQTT HELPER FUNCTIONS ===================
  // OBSOLETE 
// void generate_topic_names_from_line (uint8_t lineNum) {
//     // make strings
//     snprintf(machineID, sizeof(machineID), "machine-%d", lineNum);
//     snprintf(current_line_buffer, sizeof(current_line_buffer), "outer-line-%d", lineNum);
//     snprintf(stop_topic_buffer, sizeof(stop_topic_buffer), "thermos/data/outer-line-%d/esp/stop", lineNum);
//     snprintf(resume_topic_buffer, sizeof(resume_topic_buffer), "thermos/data/outer-line-%d/esp/resume", lineNum);
//     snprintf(bottle_count_topic_buffer, sizeof(bottle_count_topic_buffer), "thermos/raw/outer-line-%d/count", lineNum);
//     snprintf(line_status_topic_buffer, sizeof(line_status_topic_buffer), "thermos/data/outer-line-%d/esp/status", lineNum);

//     // direct pointers to buffers
//     bottle_count_topic = bottle_count_topic_buffer;
//     current_line = current_line_buffer;
//     stop_topic = stop_topic_buffer;
//     resume_topic = resume_topic_buffer;
//     line_status_topic = line_status_topic_buffer;
// }


void generate_dd_string_from_array(char *result, const char *options[], size_t option_count) {
    strcpy(result, "-\n");  // append the initial "-"
    // concatenate the rest of the strings 
    for (size_t i = 0; i < option_count; ++i) {
        if (strlen(result) + strlen(options[i]) + 2 >= MAX_OPTIONS_LENGTH) {
            printf("Warning: Dropdown string exceeded max length!\n");
            break;
        }
        strcat(result, options[i]);
        strcat(result, "\n");
    }
    // remove the last new line 
    size_t len = strlen(result);
    if (len > 0 && result[len - 1] == '\n') {
        result[len - 1] = '\0';
    }
}

void find_special_indices(int *indices, int *count, const char *reasons[], int reasons_count, const char *specials[], int specials_count) {
    *count = 0;
    for (int i = 0; i < specials_count; ++i) {
        for (int j = 0; j < reasons_count; ++j) {
            if (strcmp(specials[i], reasons[j]) == 0) {
                indices[*count] = j + 1;                  // the + 1 is due to the '-' to be appended to start of list
                (*count)++;
                break; // assuming no duplicates in reasons
            }
        }
    }
}

bool special_indices_equal(int *a, size_t a_count, int *b, size_t b_count) {
    if (a_count != b_count) {
        return false;
    }
    for (size_t i = 0; i < a_count; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// ================= SUBSCRIBE TO TOPICS ===================
void subscribe_to_topics() {
    ESP_LOGI(TAG, "Subscribing to topics...");
    esp_mqtt_client_subscribe(client, bottle_count_topic, 0);
    esp_mqtt_client_subscribe(client, line_status_topic, 0);
    esp_mqtt_client_subscribe(client, down_reason_topic, 0);
    esp_mqtt_client_subscribe(client, station_reason_topic, 0);
}

// ==================== PARSE MQTT PAYLOAD ===================
// might throw up at the number of if-elses i have in here. uek. 
void parse_mqtt_payload(const char *topic, const char *payload) { 
    Serial.printf("[PARSE] Topic: %s, Payload: %s\n", topic, payload);

    // BOTTLE COUNT 
    if (strcmp(topic, bottle_count_topic) == 0) {
        cJSON *root = cJSON_Parse(payload); 
        if (!root) {
            Serial.println("[ERROR] Failed to parse JSON (line status)");
            return;
        }

        cJSON *bottle = cJSON_GetObjectItem(root, "bottle");
        cJSON *full = cJSON_GetObjectItem(root, "full");
        bottle_count = cJSON_GetNumberValue(bottle);
        latest_full_state = cJSON_GetNumberValue(full);
        Serial.printf("[UPDATE] Bottle Count: %d\n", bottle_count);
        xQueueSend(countQueue, &bottle_count, pdMS_TO_TICKS(10));       // send count value to LVGL task

        cJSON_Delete(root);

    // LINE STATUS 
    } else if (strcmp(topic, line_status_topic) == 0) {
        cJSON *root = cJSON_Parse(payload);
        if (!root) {
            Serial.println("[ERROR] Failed to parse JSON (line status)");
            return;
        }

        cJSON *id = cJSON_GetObjectItem(root, "id");
        cJSON *status = cJSON_GetObjectItem(root, "status");
        cJSON *auto_stop = cJSON_GetObjectItem(root, "auto_stop");
        labels new_machine_state; 

        // update machine state 
        if (cJSON_IsString(status)) {
            char status_upper[32];
            
            strncpy(status_upper, status->valuestring, sizeof(status_upper) -1); 
            status_upper[sizeof(status_upper) - 1] = '\0';
            // convert to uppercase for comparison 
            for (int i = 0; status_upper[i]; i++) {
                status_upper[i] = toupper((unsigned char)status_upper[i]);
            }
            // assign machine state based on obtained string value
            new_machine_state = LABEL_COUNT; // invalid label by default
            for (int i = 0; i < LABEL_COUNT; i++) {
              if (strcmp(status_upper, label_strings[i]) == 0) {
                new_machine_state = (labels)i;
                break;
              } 
            }
            
            if (new_machine_state != LABEL_COUNT) {
                Serial.printf("[UPDATE] Machine State: %s (%d)\n", label_strings[new_machine_state], new_machine_state);
                xQueueSend(statusQueue, &new_machine_state, pdMS_TO_TICKS(10));       // send status to queue
            } else {
                Serial.printf("[ERROR] Unknown machine state: %s\n", status_upper);
            }

        } else {
            Serial.println("[ERROR] Missing or invalid 'status' field in JSON");
        }

        if (cJSON_IsNumber(auto_stop)) {
            auto_stop_flag = (int) cJSON_GetNumberValue(auto_stop);
            Serial.printf("[UPDATE] Auto-Stop: %d\n", auto_stop_flag);
        } else {
            Serial.println("[ERROR] Missing or invalid 'auto_stop' field in JSON");
        }



        cJSON_Delete(root);

    // DOWN REASONS
    } else if (strcmp(topic, down_reason_topic) == 0) {

        cJSON *root = cJSON_Parse(payload);
        if (!root) {
            printf("[ERROR] Failed to parse JSON array payload\n");
            return;
        }

        cJSON *reasons_arr = cJSON_GetObjectItem(root, "reason");
        cJSON *specials_arr = cJSON_GetObjectItem(root, "special");
        if (!cJSON_IsArray(reasons_arr) || !cJSON_IsArray(specials_arr)) {
            printf("[ERROR] Payload is not a JSON array\n");
            cJSON_Delete(root);
            return;
        }

        const char *reasons[MAX_OPTIONS_COUNT];
        const char *specials[MAX_OPTIONS_COUNT];
        size_t reasons_count = 0;
        size_t specials_count = 0;
        cJSON *item;
        cJSON_ArrayForEach(item, reasons_arr) {
            if (cJSON_IsString(item) && (reasons_count < MAX_OPTIONS_COUNT)) {
                reasons[reasons_count++] = item->valuestring;
            }
        }
        // specials cant be more than the options provvided 
        cJSON_ArrayForEach(item, specials_arr) {
            if (cJSON_IsString(item) && (specials_count < reasons_count)) {
                specials[specials_count++] = item->valuestring;
            }
        }

        cJSON_Delete(root);
        if (reasons_count == 0) {
            printf("[ERROR] No valid options found in payload\n");
            return;
        }

        char dd_options[MAX_OPTIONS_LENGTH];
        find_special_indices(latest_special_indices, &latest_special_indices_count, reasons, reasons_count, specials, specials_count);
        generate_dd_string_from_array(dd_options, reasons, reasons_count);

 
        if (xQueueSend(downoptionsQueue, dd_options, portMAX_DELAY) != pdPASS) {
            Serial.println("[ERROR] Failed to send options string to queue");
        } else {
            Serial.printf("[UPDATE] Down Reasons List: %s\n", dd_options);
        }
        
    // STATION REASONS 
    } else if (strcmp(topic, station_reason_topic) == 0) {
        cJSON *root = cJSON_Parse(payload);
        if (!root) {
            printf("[ERROR] Failed to parse JSON array payload\n");
            return;
        }

        if (!cJSON_IsArray(root)) {
            printf("[ERROR] Payload is not a JSON array\n");
            cJSON_Delete(root);
            return;
        }

        const char *options[MAX_OPTIONS_COUNT];
        size_t option_count = 0;
        cJSON *item;
        cJSON_ArrayForEach(item, root) {
            if (cJSON_IsString(item) && (option_count < MAX_OPTIONS_COUNT)) {
                options[option_count++] = item->valuestring;
            }
        }

        cJSON_Delete(root);
        if (option_count == 0) {
            printf("[ERROR] No valid options found in payload\n");
            return;
        }

        char dd_options[MAX_OPTIONS_LENGTH];
        generate_dd_string_from_array(dd_options, options, option_count);

        // send copy of value to queue 
        if (xQueueSend(stationoptionsQueue, dd_options, portMAX_DELAY) != pdPASS) {
            Serial.println("[ERROR] Failed to send options string to queue");
        } else {
            Serial.printf("[UPDATE] Station Reasons List: %s\n", dd_options);
        }
        
    }
}

// ================ NTP SETUP =================== 
void ntp_sync() {
    ESP_LOGI(TAG, "Starting NTP sync...");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "my.pool.ntp.org");   // Malaysia NTP pool
    sntp_setservername(1, "time.google.com");   // Google NTP
    sntp_setservername(2, "asia.pool.ntp.org"); // Asia NTP pool
    ESP_LOGI("NTP", "Attempting time sync...");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (retry < 10) {
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year > (2020 - 1900)) {
          ESP_LOGI(TAG, "NTP sync successful!");
          setenv("TZ", "SGT-8", 1); //+08:00
          tzset();

          time(&now);
          localtime_r(&now, &timeinfo);
          ESP_LOGI(TAG, "Time synced: %s", asctime(&timeinfo));
          return;
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        retry++;
    }
    ESP_LOGI(TAG, "NTP sync failed.");
}


// ============ WIFI, MQTT EVENT HANDLERS =================
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_connected = true;
        ESP_LOGI(TAG, "Connected to WiFi");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        ESP_LOGI(TAG, "Disconnected! Reconnecting...");
        esp_wifi_connect();
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            Serial.println("[MQTT] Connected to broker");
            subscribe_to_topics();
            break;

        case MQTT_EVENT_DATA:
            char topic[128];
            char payload[128];
            snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
            snprintf(payload, sizeof(payload), "%.*s", event->data_len, event->data);

            Serial.printf("[MQTT] Received topic: %s, payload: %s\n", topic, payload);

            parse_mqtt_payload(topic, payload);
            break;

        default:
            break;
    }
}


// ============ WIFI MQTT SETUP =====================
void init_wifi_mqtt() {
  ESP_ERROR_CHECK(nvs_flash_init()); // Required for WiFi
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));

  wifi_config_t wifi_config = {};
  strcpy((char*)wifi_config.sta.ssid, ssid);
  strcpy((char*)wifi_config.sta.password, password);

  /*
  wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
  */

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker = {
        .address = {
          .uri = mqtt_broker,
        }
      }
  };
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);
}


// ========== FREERTOS TASKS ========================

// publish something
void mqttTask(void *pvParameters) {
    PubMessage msg;
    while (1) {
        if (xQueueReceive(publishQueue, &msg, portMAX_DELAY)) {
            esp_mqtt_client_publish(client, msg.topic, msg.payload, 0, msg.qos, msg.retain);
            ESP_LOGI("MQTT_PUBLISH", "Published to %s: %s", msg.topic, msg.payload);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to avoid CPU overload
    } 
}

// update UI from queue, (with very small taskdelay)
void lvglTask(void *pvParameters) {

    int receivedValue;

    char receivedDownOptions[MAX_OPTIONS_LENGTH];
    char receivedStationOptions[MAX_OPTIONS_LENGTH];
    while (1) {
        
        // BOTTLE COUNT 
        if (xQueueReceive(countQueue, &receivedValue, pdMS_TO_TICKS(10))) {
        
            lv_label_set_text(objects.count, String(receivedValue).c_str());                            // update disp counter

            if (latest_full_state != current_full_state) {                                                                           // update counter panel colour
              lv_obj_set_style_bg_color(objects.midpanel, lv_color_hex(get_color_hex(FULL)), LV_PART_MAIN);
              current_full_state = latest_full_state;
            } else {
              lv_obj_set_style_bg_color(objects.midpanel, lv_color_hex(get_color_hex(NOT_FULL)), LV_PART_MAIN);
            }
        } 

        // STATUS UPDATE 
        if (xQueueReceive(statusQueue, &latest_machine_state, pdMS_TO_TICKS(10))) {
        
            if (latest_machine_state != current_machine_state) {                                        // update status label
              current_machine_state = latest_machine_state;     // reassign machine state
              set_top_panel_status(objects.toppanel, objects.statuslabel, (labels)current_machine_state);
              button_check(objects.btn1, objects.btn2, (labels)current_machine_state);
            }

            // i dah malas but i think might want to move the auto_stop update here 
        } 


        // DOWN REASON OPTIONS (store latest instead of updating directly);
        if (xQueueReceive(downoptionsQueue, receivedDownOptions, pdMS_TO_TICKS(10))) {
            // lv_dropdown_set_options(objects.dd1, receivedOptions);
            strncpy(latest_down_reason, receivedDownOptions, MAX_OPTIONS_LENGTH - 1);
            latest_down_reason[MAX_OPTIONS_LENGTH - 1] = '\0';  // cuz strncpy doesnt copy over the null char
            if (strcmp(current_down_reason, latest_down_reason) != 0) {
              ddd_update_required = true;
              Serial.println("[LVGL] Latest down reason update required.");
            } else {
              ddd_update_required = false;                        
              Serial.println("[LVGL] Down reasons up to date."); 
            }
        }

        // STATION REASON OPTIONS (store latest instead of updating directly);
        if (xQueueReceive(stationoptionsQueue, receivedStationOptions, pdMS_TO_TICKS(10))) {
            strncpy(latest_station_reason, receivedStationOptions, MAX_OPTIONS_LENGTH - 1);
            latest_station_reason[MAX_OPTIONS_LENGTH - 1] = '\0';  // cuz strncpy doesnt copy over the null char
            if (strcmp(current_station_reason, latest_station_reason) != 0 || 
                !special_indices_equal(current_special_indices, current_special_indices_count, latest_special_indices, latest_special_indices_count)) {
              sdd_update_required = true;
              Serial.println("[LVGL] Latest station reason update required.");
            } else {
              sdd_update_required = false;                        
              Serial.println("[LVGL] Station reasons up to date."); 
            }
        }

        vTaskDelay(20 / portTICK_PERIOD_MS); // Small delay to avoid CPU overload
    }

}

// initiate sync upon wifi connection 
void ntpTask(void *pvParameters) {
    while (!wifi_connected) {
        Serial.println("WiFi not connected"); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.println("Connected to WiFi");
    ntp_sync();
    xTaskCreatePinnedToCore(update_time_task, "Update Time", 4096, NULL, 1, NULL, 1);
    vTaskDelete(NULL);          
}

// updates current time 
void update_time_task(void *pvParameters) {
    while (1) {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year > (2020 - 1900)) {
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
            printf("Time: %s\n", time_str);
            // lv_label_set_text(time_label, time_str);   // in case the time thing is needed
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Update every second
    }
}

// =========== LCD PANEL UI INIT ==================
void init_lvgl()
{
    String title = "LVGL UI";

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
// -----------------------------------------------------
    /* Release the mutex */
    lvgl_port_unlock();

    Serial.println(title + " end");
}


//======== SETUP TASK ========== 
void setup(){
  Serial.begin(115200);
  init_lvgl(); 
  // generate_topic_names_from_line(1);                  // LINE 1
  init_wifi_mqtt(); 

  countQueue = xQueueCreate(5, sizeof(int));                // queue of 5 integers 
  downoptionsQueue = xQueueCreate(5, MAX_OPTIONS_LENGTH);                      // queue of 5 strings, max length 256 
  stationoptionsQueue = xQueueCreate(5, MAX_OPTIONS_LENGTH);                      // queue of 5 strings, max length 256 
  statusQueue = xQueueCreate(5, sizeof(int));                      // queue of 5 strings, max length 256 
  publishQueue = xQueueCreate(5, sizeof(PubMessage));       // queue of 5 structs
  if (publishQueue == NULL) {
      ESP_LOGE(TAG, "Failed to create publish queue");
  }
  
  // all in same priority, will run in turns
  xTaskCreatePinnedToCore(lvglTask, "LVGL Task", 4096, NULL, 1, NULL, 1);       // pin lvgl task to core 1
  xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 1, NULL, 0);       // pin mqtt task to core 0 
  xTaskCreatePinnedToCore(ntpTask, "NTP Task", 4096, NULL, 1, NULL, 0);         // pin ntp task to core 0 
}

// ======= LOOP TASK ============
void loop() {
  // just an incrementing counter for task
  counter = (counter < 255) ? counter + 1 : 0;
  // xQueueSend(queue, &counter, pdMS_TO_TICKS(10));     // send LOCAL counter value to queue 
  delay(1000);
}

