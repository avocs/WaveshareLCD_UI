#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cJSON.h>
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

extern QueueHandle_t publishQueue;
void queue_mqtt_publish(const char *topic, const char *payload, int qos, bool retain);
char *generate_report_json(const char *reason, const char *specReason, const char *remark);


#ifdef __cplusplus
}
#endif

#endif // MQTT_PUBLISHER_H