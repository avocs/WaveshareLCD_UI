#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>


// ======= SETTINGS ======================
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

const char *ssid = "Tanand_Hardware";
const char *password = "202040406060808010102020";

const char *mqtt_broker = "192.168.0.101";
const int mqtt_port = 1883;

// MQTT TOPICS
const char *rand_topic = "random/test/esp32"; 
const char *current_line; 
const char *stop_topic = "thermos/esp/stop"; 
const char *resume_topic = "thermos/esp/resume"; 
const char *bottle_count_topic = "thermos/esp/count"; 
const char *line_status_topic = "thermos/esp/status"; 
const char *down_reason_topic = "thermos/esp/downreason"; // fixed topic 
const char *station_reason_topic = "thermos/esp/stationreason"; // fixed topic 



unsigned long lastBottleTime = 0;
unsigned long lastReasonTime = 0;
unsigned long lastStatusTime = 0;

const unsigned long bottleInterval = 1000;  
const unsigned long reasonInterval = 5000;  
const unsigned long statusInterval = 7000;  

// dummy variables for simulation 
char strBuffer[128]; 
uint8_t bottle_count = 0; 
uint8_t full = 0;
uint8_t auto_stop = 1;

String str, str2, str3; 

// ============== FUNCTION PROTOTYPES ===================
void mqttCallback(char *topic, uint8_t *payload, unsigned int length);
void connect_to_MQTT_broker();
void mqtt_setup();

// ================ MQTT HELPER FUNCTIONS ===================
// void generate_topic_names_from_line (uint8_t lineNum) {
//     // make strings
//     snprintf(current_line_buffer, sizeof(current_line_buffer), "outer-line-%d", lineNum);
//     snprintf(stop_topic_buffer, sizeof(stop_topic_buffer), "thermos/data/outer-line-%d/esp/stop", lineNum);
//     snprintf(resume_topic_buffer, sizeof(resume_topic_buffer), "thermos/data/outer-line-%d/esp/resume", lineNum);
//     snprintf(bottle_count_topic_buffer, sizeof(bottle_count_topic_buffer), "thermos/raw/outer-line-%d/count", lineNum);
//     snprintf(line_run_status_topic_buffer, sizeof(line_run_status_topic_buffer), "thermos/data/outer-line-%d/esp/status", lineNum);

//     // direct pointers to buffers
//     bottle_count_topic = bottle_count_topic_buffer;
//     current_line = current_line_buffer;
//     stop_topic = stop_topic_buffer;
//     resume_topic = resume_topic_buffer;
//     line_run_status_topic = line_run_status_topic_buffer;
// }



// ======== WIFI CONNECTION ============
void connect_to_wifi() {

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network!");

}

// ======== MQTT SETUP =========
void mqtt_setup() {
  mqtt_client.setBufferSize(4096);                   // the default is 256, not enough!
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  connect_to_MQTT_broker();

}

// callback triggered when message received from broker
void mqttCallback(char *topic, uint8_t *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  // Serial.print("Message:");
  // for (unsigned int i = 0; i < length; i++) {
  //     Serial.print(payload[i]);
  // }
  Serial.println("-----------------------");
}

void mqtt_loop() {
  if (!mqtt_client.connected()) {
      connect_to_MQTT_broker();
  }
  mqtt_client.loop();         // loop that continuously tries to keep the connection alive 
}

// connect to a broker and subscribe to topic via tcp 
void connect_to_MQTT_broker() {
  while (!mqtt_client.connected()) {
      String client_id = "esp32-client";
      Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
      if (mqtt_client.connect(client_id.c_str())) {
          Serial.println("Connected to MQTT broker!");

          // subscribe to a topic 
          // mqtt_client.subscribe(mqtt_topic_frame);
          // Publish message upon successful connection
          mqtt_client.subscribe(bottle_count_topic);
          mqtt_client.publish(rand_topic, "Connected from ESP!");
      
      // retrying the loop 
      } else {
          Serial.print("Failed to connect to MQTT broker, rc = ");
          Serial.println(mqtt_client.state());   
          Serial.println("Trying again in 5 seconds...");
          delay(5000);
      }
  }
}


// ======== GENERATE DROP DOWN LIST ====== 
String generate_generic_drop_down_list() {
  JsonDocument doc; 
  JsonArray choices = doc["reason"].to<JsonArray>();  
  JsonArray specials = doc["special"].to<JsonArray>();  

  choices.add("Choice 1"); 
  choices.add("Choice 2"); 
  choices.add("Choice 3"); 
  choices.add("Choice 4"); 
  choices.add("Choice 5"); 

  specials.add("Choice 4");
  specials.add("Choice 5");
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr; 
}

String generate_special_drop_down_list() {
  JsonDocument doc; 
  JsonArray choices = doc.to<JsonArray>();  

  choices.add("Choice a"); 
  choices.add("Choice b"); 
  choices.add("Choice c"); 
  choices.add("Choice d"); 
  choices.add("Choice e"); 
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr; 
}

String generate_status_update(const char* id, const char* status, int auto_stop) {
  JsonDocument doc; 

  doc["id"] = id;
  doc["status"] = status; 
  doc["auto_stop"] = 1; 
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr; 
}


String generate_bottle_update(int value, int full) {
  JsonDocument doc; 

  doc["bottle"] = value;
  doc["full"] = full; 
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  return jsonStr; 
}


// ==== MAIN SETUP AND LOOP ======= 
void setup() {
  Serial.begin(115200); 
  str = generate_generic_drop_down_list();
  str2 = generate_special_drop_down_list();
  connect_to_wifi();
  mqtt_setup();

}

void loop() {
  // put your main code here, to run repeatedly:
  mqtt_loop();
  unsigned long currentMillis = millis();

  if (currentMillis - lastBottleTime >= bottleInterval) {
    lastBottleTime = currentMillis; 
    bottle_count ++;
    (bottle_count % 20 == 0) ? full = 1 : full = 0; 
    String strBottle = generate_bottle_update(bottle_count, full);
    mqtt_client.publish(bottle_count_topic, strBottle.c_str());  
  }


  if (currentMillis - lastReasonTime >= reasonInterval) {
    lastReasonTime = currentMillis; 
    mqtt_client.publish(down_reason_topic, str.c_str());  
    mqtt_client.publish(station_reason_topic, str2.c_str());    
  }


  if (currentMillis - lastStatusTime >= statusInterval) {
    lastStatusTime = currentMillis; 
    String strStatus = generate_status_update("Outer line 1", "Stop", auto_stop);
    mqtt_client.publish(line_status_topic, strStatus.c_str());  
  }


  
}

