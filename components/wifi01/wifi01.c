
#include "wifi01.h"
#include "wifi_key.h"

// Not required anymore, converted into parameters
//#define WIFI_AUTORECONNECT      true                          /* flag to configure wifi autoreconnection */
//#define SNTP_SYNC_ACTIVE        true                          /* flag to activate/deactivate sntp sync*/

//#define SNTP_UPDATE_DELAY    60000                          /* DELAY >= 15000 according to standard */ 
//#define SNTP_UPDATE_DELAY  3600000                          /* SNTP time update: 1h.    */
#define SNTP_UPDATE_DELAY   43200000                          /* SNTP time update: 1 day */

//TODO: Learn how to check if the loop is already initialized and improve code
static bool wifi_event_loop_started = false;                // variable used to initialize the loop only the first time
static bool wifi_auto_reconnect = true;                 // reconnect if connection is lost (deactivate when stopping process)
static bool sntp_sync_active = true;                    // activate SNTP service update

static bool wifi_connected = false;                     // activated and deactivated by events

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

static const char *TAG = "WIFI01";


/****************************************************************************** 
* wifi activate
*******************************************************************************
 * @brief activates wifi in STA mode. 
 * @param[in] auto_reconnect = true -> Autoreconnect if connection is lost 
 * @param[in] sntp_sync = true -> connect to snmt servers and get time
*******************************************************************************/
void wifi_activate(bool auto_reconnect, bool sntp_sync)
{
    wifi_auto_reconnect = auto_reconnect;
    sntp_sync_active = sntp_sync;

    // Create these structures the first call. They are not deleted when wifi is stopped.
    if (!wifi_event_loop_started) {
        //wifi_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
        wifi_event_loop_started = true;
        ESP_ERROR_CHECK( nvs_flash_init() );        
    }

    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    // TODO: use parameters to send wifi usr and password
    static wifi_config_t wifi_config;
    strcpy(&wifi_config.sta.ssid,  &EXAMPLE_WIFI_SSID);
    strcpy(&wifi_config.sta.password, &EXAMPLE_WIFI_PASS);
    wifi_config.sta.bssid_set = 0;

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );                            
}



/****************************************************************************** 
* wifi deactivate
*******************************************************************************
 * @brief deactivates wifi in STA mode. 
 * @brief SNTP service is also stopped.
*******************************************************************************/
void wifi_deactivate(void)
{
    wifi_auto_reconnect = false;                //deactivate auto reconnection before stopping sntp
    sntp_stop();                                //validates internally if sntp service is started
    ESP_ERROR_CHECK( esp_wifi_stop() );         //tcpip_adapter and DHCP server/client are automatically stopped.

}


/****************************************************************************** 
* wifi reconnect
*******************************************************************************
 * @brief reconnects wifi in case of disconnection 
 * @brief as when SYSTEM_EVENT_STA_DISCONNECTED is received.
 * @brief sntp must be disconnected after trying to reconnect wifi.
*******************************************************************************/
void wifi_reconnect(void){
    // bucle para reconnectar N veces
    // Corregir para lanzar un hilo y que sea independiente del proceso donde se gestiona el evento

    int multiplo = 60;
    int retry_nbr_inc = 10;
    int retry_nbr_limit = 100;
    int retry_nbr = 0;

    do {
        wifi_deactivate();
        ESP_LOGI(TAG, "Wifi deactivate");
        vTaskDelay(pdMS_TO_TICKS(1000));
        wifi_activate(true, true);
        ESP_LOGI(TAG, "Wifi reconnect");
        vTaskDelay(pdMS_TO_TICKS( (1 + multiplo * retry_nbr) * 1000));
        if (retry_nbr < retry_nbr_inc) { retry_nbr++; };
        } while (!wifi_connected || retry_nbr < retry_nbr_limit);
}





/****************************************************************************** 
* wifi deactivate
*******************************************************************************
 * @brief deactivates wifi in STA mode. 
 * @brief SNTP service is also stopped.
*******************************************************************************/
void sntp_start(void)
{
    // TODO: Verify that wifi service is operative before calling it if the function is not static
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

/****************************************************************************** 
* sntp stop
*******************************************************************************
 * Use sntp_stop() function, already exists.
*******************************************************************************/



/****************************************************************************** 
* wifi event handler
*******************************************************************************
 * some asyncrhonous wifi actions are handled though events, here: 
*******************************************************************************/
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    esp_err_t error;
    switch(event->event_id) {
        
    case SYSTEM_EVENT_STA_START:
        ESP_ERROR_CHECK( esp_wifi_connect() );
        ESP_LOGI(TAG, "Event: SYSTEM_EVENT_STA_START -> Received");        
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        wifi_connected = true;
        ESP_LOGI(TAG, "Event: SYSTEM_EVENT_STA_CONNECTED -> Received");
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "Event: SYSTEM_EVENT_STA_GOT_IP: " IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
        if (sntp_sync_active){
            sntp_start();
            ESP_LOGI(TAG, "SNTP: Initialized");
        }
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        wifi_connected = false;
        ESP_LOGI(TAG, "Event: SYSTEM_EVENT_STA_DISCONNECTED -> Received");
        /* Workaround as ESP32 WiFi libs don't currently auto-reassociate. */
        if (wifi_auto_reconnect) {
            wifi_reconnect();
        }
        break;

    case SYSTEM_EVENT_STA_STOP:
        ESP_LOGI(TAG, "Event: SYSTEM_EVENT_STA_STOP -> Received");        
        break;


    default:
        ESP_LOGI(TAG, "Default Event: %d ", event->event_id);
        break;
    }
    return ESP_OK;
}
