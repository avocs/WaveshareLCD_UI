#include "mqtt_publisher.h"
#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "MQTT_PUBLISHER";

QueueHandle_t publishQueue;

typedef struct {
    char topic[64];
    char payload[MAX_PAYLOAD_LENGTH];
    int qos;
    bool retain;
} PubMessage;

void queue_mqtt_publish(const char *topic, const char *payload, int qos, bool retain) {
    // convert the topic and payload into a singular struct
    PubMessage msg;
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';

    strncpy(msg.payload, payload, sizeof(msg.payload) - 1);
    msg.payload[sizeof(msg.payload) - 1] = '\0';

    msg.qos = qos;
    msg.retain = retain;

    if (xQueueSend(publishQueue, &msg, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "Publish queue full, message dropped!");
    }
}

char *generate_report_json(const char *reason, const char *specReason, const char *remark) {
    cJSON *root = cJSON_CreateObject();

    if (root == NULL) {
        return NULL; // Memory allocation failed
    }

    cJSON_AddStringToObject(root, "reason", reason);
    cJSON_AddStringToObject(root, "station", specReason);
    cJSON_AddStringToObject(root, "des", remark);

    char *jsonString = cJSON_PrintUnformatted(root);  // Allocate string
    cJSON_Delete(root);                               // Free cJSON object

    return jsonString;  // Caller is responsible for freeing this string
}

