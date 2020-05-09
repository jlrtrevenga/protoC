/* esp_event (event loop library) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef MOD_MQTT_H_
#define MOD_MQTT_H_

/*
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_timer.h"
#include "sensor.h"
*/
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
esp_mqtt_client_handle_t client;
esp_err_t err;
} mqtt_client_par_t;


esp_err_t mqtt_app_start(void);
int mqtt_client_publish(const char *topic, const char *data, int len, int qos, int retain);
int mqtt_client_subscribe(const char *topic, int qos);





#ifdef __cplusplus
}
#endif

#endif // #ifndef MOD_MQTT_H_

