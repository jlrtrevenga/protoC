/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"
//#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_tls.h"
#include "mod_mqtt.h"
#include "mqtt_client.h"


static const char *TAG = "MOD_MQTT";

/*
#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipse_org_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipse_org_pem_start[]   asm("_binary_mqtt_eclipse_org_pem_start");
#endif
extern const uint8_t mqtt_eclipse_org_pem_end[]   asm("_binary_mqtt_eclipse_org_pem_end");
*/

static esp_mqtt_client_handle_t client;
static bool mqtt_connected = false;         // 

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGV(TAG, "MQTT_EVENT_DATA");
            ESP_LOGV(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGV(TAG, "TOPIC=%.*s\r\n", event->data_len, event->data);            
            //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
                
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}



/****************************************************************************** 
* mqtt_app_start
*******************************************************************************
 * @brief starts mqtt client. 
*******************************************************************************/
 esp_err_t mqtt_app_start(void)
{

/*
   const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URI,
        .cert_pem = (const char *) mqtt_eclipse_org_pem_start,
    };
*/
  
   const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URI,
    };
   
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    //esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_err_t err = esp_mqtt_client_start(client);
    return (err);
}



/****************************************************************************** 
* mqtt_client_publish in active mqtt client
*******************************************************************************
 * @brief starts mqtt client. 
 * @param[in] *topic -> topic in which we write  
 * @param[in] *data -> published data 
 * @param[in] len   -> data lenght
 * @param[in] qos   -> quality of service (0,1,2)
 * @param[in] retain -> retain 
*******************************************************************************/
int mqtt_client_publish(const char *topic, const char *data, int len, int qos, int retain){
    int msg_id = esp_mqtt_client_publish(client, topic, data, len, qos, retain);
    return(msg_id);
}


/****************************************************************************** 
* mqtt_client_subscribe in active mqtt client
*******************************************************************************
 * @brief starts mqtt client. 
 * @param[in] *topic -> topic in which we write  
 * @param[in] qos   -> quality of service (0,1,2)
*******************************************************************************/
int mqtt_client_subscribe(const char *topic, int qos){
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    return(msg_id);    
}



/*
msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
*/ 

