#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

#include <lvgl.h>
#include "lvgl_port_v8.h"
// #include <demos/lv_demos.h>
// #include <examples/lv_examples.h>

#include <ui.h> 
// #include <actions.h>

// 
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
const char *mqtt_broker = "mqtt://192.168.0.103/";
const char *topic       = "random/test/esp32s3lcd-1";
const char *r_topic     = "random/test/send";
const int mqtt_port     = 1883;
static const char* TAG  = "ESP32";

// Counter
int counter             = 0;
float data              = 0.0;
bool wifi_connected     = false;
esp_mqtt_client_handle_t client;
QueueHandle_t queue, mq_data;

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
            ESP_LOGI(TAG, "MQTT connected!");
            esp_mqtt_client_subscribe(client, r_topic, 0);
            break;

        case MQTT_EVENT_DATA:
            char msg[20];
            char *endptr;
            snprintf(msg, sizeof(msg), "%.*s", event->data_len, event->data);
            Serial.printf("Received: %s\n", msg);
            // data = atof(msg);
            data = strtof(msg, &endptr);
            if (*endptr != '\0') data = 0.0f;
            xQueueSend(mq_data, &data, pdMS_TO_TICKS(10));
            break;

        default:
            break;
    }
}

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


// ------ FREERTOS TASKS ----------
// publish something
void mqttTask(void *pvParameters) {
    while (1) {
      esp_mqtt_client_publish(client, topic, "hello from waveshare", 0, 0, false);
      vTaskDelay(pdMS_TO_TICKS(5000));  // MQTT runs in background, no need for a loop
    }
}

// update UI from queue
void lvglTask(void *pvParameters) {

  int receivedValue;
  float rec_mq_data;
    while (1) {
        if (xQueueReceive(queue, &receivedValue, pdMS_TO_TICKS(10))) {
            Serial.print("Received Value Local: ");
            Serial.println(receivedValue);
            lv_label_set_text(objects.count, String(receivedValue).c_str());
            // lv_obj_set_style_text_font(test_label, &lv_font_montserrat_14, 0);
            // lv_obj_align(test_label, LV_ALIGN_CENTER, 0, 0);
            // if (receivedValue == 0) lv_obj_set_style_bg_color(objects.midpanel, lv_color_hex(0xfad2c3), LV_PART_MAIN); 
            // else lv_obj_set_style_bg_color(objects.midpanel, lv_color_hex(0xffffff), LV_PART_MAIN); 
        }
        if (xQueueReceive(mq_data, &rec_mq_data, pdMS_TO_TICKS(10))) {
            Serial.print("Received Value MQTT: ");
            Serial.println(rec_mq_data);
            String display = "Server:  " + String(rec_mq_data);
            // lv_label_set_text(mqtt_label, display.c_str());
            // lv_obj_set_style_text_font(mqtt_label, &lv_font_montserrat_64, 0);
            // lv_obj_set_style_text_font(mqtt_label, &lv_font_montserrat_14, 0);
            // lv_obj_align(mqtt_label, LV_ALIGN_BOTTOM_MID, 0, 0);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to avoid CPU overload
    }
  
}

//initiate sync upon wifi connection 
void ntpTask(void *pvParameters) {
    while (!wifi_connected) {
        Serial.println("WiFi not connected"); 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.println("Connected to WiFi");
    ntp_sync();
    xTaskCreatePinnedToCore(update_time_task, "Update Time", 4096, NULL, 1, NULL, 1);
    vTaskDelete(NULL);          // does this delete itself, yeah i think it does 
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

// ------- Create UI  ---------
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


// ------ SETUP TASK ---------- 
void setup(){
  Serial.begin(115200);
  init_lvgl(); 
  init_wifi_mqtt(); 

  queue = xQueueCreate(5, sizeof(int));               // queue of 5 integers 
  mq_data = xQueueCreate(5, sizeof(float));
  
  // all in same priority, will run in turns
  xTaskCreatePinnedToCore(lvglTask, "LVGL Task", 4096, NULL, 1, NULL, 1);       // pin lvgl task to core 1
  xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 1, NULL, 0);       // pin mqtt task to core 0 
  xTaskCreatePinnedToCore(ntpTask, "NTP Task", 4096, NULL, 1, NULL, 0);         // pin ntp task to core 0 
}

// ------- LOOP TASK -----------
void loop() {
  // just an incrementing counter for task
  counter = (counter < 255) ? counter + 1 : 0;
  xQueueSend(queue, &counter, pdMS_TO_TICKS(10));     // send counter value to queue 
  delay(1000);
}

