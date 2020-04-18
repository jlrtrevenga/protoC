

/* Heater Control Test. Prototype B 

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_log.h>
#include <esp_event_base.h>
#include <esp_event.h>
#include "heater_ctrl.h"
#include "esp_idf_lib_helpers.h"
#include "bmp280_ctrl_loop.h"
#include "sensor.h"
#include "bmp280.h"
#include "wifi01.h"
//#include "sensor.h"
//#include "driver/gpio.h"

#define DELAY_1s             (pdMS_TO_TICKS( 1000))
#define DELAY_2s             (pdMS_TO_TICKS( 2000))
#define DELAY_5s             (pdMS_TO_TICKS( 5000))
#define DELAY_60s             (pdMS_TO_TICKS( 60000))
#define DELAY_15m             (pdMS_TO_TICKS( 900000))
#define DELAY_1h             (pdMS_TO_TICKS( 3600000))


#define DEBUG_EVENTS 0              // 0:NO DEBUG , 1:DEBUG

//GPIO DEFINITION
//#define GPIO_INPUT_IO_0     12
//#define GPIO_INPUT_IO_1     14
//#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))// I2C GPIO

//I2C GPIO
#define SDA_GPIO 21
#define SCL_GPIO 22


static const char* TAG = "protoC";

// Events loop
esp_event_loop_handle_t event_loop_h;


int deadband_check(measure_t measure, measure_t setpoint, float deadband);

//***************************************************************************** 
//main task
//*****************************************************************************
void app_main()
{

    // DEFINIR LOS NIVELES DE LOG POR TAG
    esp_log_level_set("BMP280_CTRL_LOOP", 1);
    esp_log_level_set("HEATER_CTRL", 3);        // tiene compilacion condicional para errores
    esp_log_level_set("wifi", 1);
    esp_log_level_set("event", 1);
    esp_log_level_set("WIFI01", 3);
    esp_log_level_set("TASK_PROGRAMMER01", 1);
    esp_log_level_set("WIFI_EXAMPLE", 3);           // REMOVE? CHECK
    esp_log_level_set("protoC", 3);

	// time setting
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    //char strftime_buf[64];

    // Configure timezone
    // TODO: Choose from screen/parameters
    // Set timezone to SPAIN Standard Time
    // TODO: Although it should be UTC+1, it seems we have to use UTC-1 (1 hour west from Grenwich, to make it work fine)
    setenv("TZ", "UTC-2,M3.5.0/2,M10.4.0/2", 1);
    tzset();

    // TODO: esto es prueba rapida, codificar bien mas adelante, sacar como var globales junto con todas las DOs
    static DO_t heater_command;
    heater_command.value_actual = 0;
    heater_command.value_prev = 0;


    // 
    ESP_LOGI(TAG, "event loop setup");

    // 1.- COMMON SERVICES

    // COMMON EVENT LOOP
    esp_event_loop_args_t event_loop_args = {
        .queue_size = 5,
        .task_name = "event_loop_task",                 // task will be created (implicit)
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 2048,
        .task_core_id = tskNO_AFFINITY
    };

    // Create the event loops
    ESP_ERROR_CHECK(esp_event_loop_create(&event_loop_args, &event_loop_h));
    ESP_LOGI(TAG, "event loop created");

    // COMMON I2C "services" (mutex access control)
    ESP_ERROR_CHECK(i2cdev_init());


	// Activate wifi and get time via NTP Server
	// TODO: Start/stop Connectivity (WIFI / GPRS/OTHERS)
	// TODO: Validate time is correct before activating programmer
       wifi_activate(true, true);


    // 2.- SERVICES LOOPS

    // 2.1.- heater_ctrl task loop: Init "heater control loop parameters" and create task
    
    measure_t temperature_setpoint;

    heater_ctrl_loop_params_t heater_ctrl_loop_params;
    heater_ctrl_loop_params.event_loop_handle = event_loop_h;
    heater_ctrl_loop_params.ulLoopPeriod = 1000;
    heater_ctrl_loop_params.pxTaskHandle = NULL;
    heater_ctrl_loop_params.pxtemperature = &temperature_setpoint;

    heater_ctrl_loop_params_t* pxheater_ctrl_loop_params = NULL;
    pxheater_ctrl_loop_params = &heater_ctrl_loop_params;

    //static const char *pxTask01parms = "Task 1 is running\r\n"; 
    if ( xTaskCreatePinnedToCore(&heater_ctrl_loop, "heater_ctrl_loop", 1024 * 2, 
                                 (void*) pxheater_ctrl_loop_params, 5,
                                 heater_ctrl_loop_params.pxTaskHandle, APP_CPU_NUM) != pdPASS ) {    
        ESP_LOGE(TAG, "heater_ctrl task creation failed");
        } 
    else { ESP_LOGI(TAG, "heater_ctrl task created"); }


#if DEBUG_EVENTS == 1
    // 2.1.1.- heater_test task, only for testing event reception and processing
    // TODO: Remove when tested.

    if ( xTaskCreatePinnedToCore(&heater_test_loop, "heater_test_loop", 1024 * 2, 
                                NULL, 5, NULL, APP_CPU_NUM) != pdPASS ) {
        ESP_LOGE(TAG, "heater_test creation failed");
    } else {
        ESP_LOGI(TAG, "heater_test created\r\n");
    }
#endif

    // 2.2.- bmp280_ctrl task loop: Init "bmp280 control loop parameters" and create task

    BMP280_Measures_t BMP280_Measures;      // Values are updated in background by bmp280_control_loop

    BMP280_control_loop_params_t BMP280_ctrl_loop_params;
    BMP280_ctrl_loop_params.event_loop_handle = event_loop_h;
    BMP280_ctrl_loop_params.ulLoopPeriod = 1000;
    BMP280_ctrl_loop_params.pxTaskHandle = NULL;
    BMP280_ctrl_loop_params.sda_gpio = SDA_GPIO;
    BMP280_ctrl_loop_params.scl_gpio = SCL_GPIO;
    BMP280_ctrl_loop_params.pxBMP280_Measures = &BMP280_Measures;

    BMP280_control_loop_params_t* pxBMP280_ctrl_loop_params = NULL;
    pxBMP280_ctrl_loop_params = &BMP280_ctrl_loop_params;

    if ( xTaskCreatePinnedToCore(&bmp280_ctrl_loop, "bmp280_ctrl_loop", 1024 * 2, 
                                 (void*) pxBMP280_ctrl_loop_params, 5,
                                 BMP280_ctrl_loop_params.pxTaskHandle, APP_CPU_NUM) != pdPASS ) {    
        ESP_LOGE(TAG, "bmp280_ctrl task creation failed\r\n");
        } 
    else { ESP_LOGI(TAG, "bmp280_ctrl task created\r\n"); }

#if DEBUG_EVENTS == 1
    // 2.2.1.- bmp280_test task, only for testing event reception and processing
    // TODO: Remove when tested.
    if ( xTaskCreatePinnedToCore(&bmp280_test_loop, "bmp280_test_loop", 1024 * 2, 
                                 NULL, 5, NULL, APP_CPU_NUM) != pdPASS ) {
        ESP_LOGE(TAG, "bmp280_test creation failed\r\n");
    } else {
    	ESP_LOGI(TAG, "bmp280_test created\r\n");
    }
#endif


    // ENDLESS LOOP, REMOVE AND SUPRESS BY VALID CODE
    //TickType_t tick;
	for(;;) {
        TickType_t tick = xTaskGetTickCount();
		vTaskDelay(DELAY_60s);          // Definir cada minuto

        // Select temperature sensor that controls temperature and read room temperature
        // TODO, de momento me lo salto

        time_t now1;
        time(&now1);
        struct tm timeinfo;
        localtime_r(&now1, &timeinfo);

        float deadband = 2;
        int result = deadband_check(BMP280_Measures.temperature, temperature_setpoint, deadband);
        switch (result){
            case 1:     // over db => SET HEATER OFF
                heater_command.value_prev = heater_command.value_actual;
                heater_command.value_actual = 0;
                if (heater_command.value_actual != heater_command.value_prev){
                    ESP_LOGI(TAG, "%d / %d:%d SET HEATER OFF. Setpoint: %f, Temperature:%f ", 
                        timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min,
                        temperature_setpoint.value, BMP280_Measures.temperature.value);
                    }
                break;

            case 2:     // inside db => KEEP ACTUAL HEATER STATUS (ON OR OFF)
                heater_command.value_prev = heater_command.value_actual;
                //heater_command.value_actual = heater_command.value_actual;
                // TODO; Una vez que cambia, no cambiar en XXX segundos, a definir
                break;
            
            case 3:     // under db => SET HEATER ON
                heater_command.value_prev = heater_command.value_actual;
                heater_command.value_actual = 1;
                if (heater_command.value_actual != heater_command.value_prev){
                    ESP_LOGI(TAG, "%d / %d:%d SET HEATER ON. Setpoint: %f, Temperature:%f ", 
                        timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min,
                        temperature_setpoint.value, BMP280_Measures.temperature.value);
                    }
                break;

            // TODO: Casos en los que la calidad sea mala, decidir qué hacer
            // TODO: Convertir en caso general, dar mismo tratamiento a todas las salidas    
            }



        if ((timeinfo.tm_min%15) == 0){

            ESP_LOGI(TAG, "%d / %d:%d Trace........ Setpoint: %3.2f ºC (%d), Temperature:%3.2f %s (%d)", 
                    timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min,
                    temperature_setpoint.value, temperature_setpoint.quality,
                    BMP280_Measures.temperature.value, BMP280_Measures.temperature.displayUnit, BMP280_Measures.temperature.quality);
            /*        
            ESP_LOGI(TAG,  "%d / %d:%d Trace........ BMP280.Temp(q= %d): %3.2f %s / BMP280.Press(q= %d): %6.2f %s", 
                timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min,
                BMP280_Measures.temperature.quality, BMP280_Measures.temperature.value, BMP280_Measures.temperature.displayUnit, 
                BMP280_Measures.pressure.quality, BMP280_Measures.pressure.value, BMP280_Measures.pressure.displayUnit);
            */
            }




		}
  
}


// TODO- Eval also quality
/***************************************************************************** 
* deadband_check
******************************************************************************
 * @brief check if the value is under/inside/over deadband
 * @param[in] value
 * @param[in] deadband
 * @return: 1:over db / 2:Inside db / 3:under db
 */
int deadband_check(measure_t measure, measure_t setpoint, float deadband){   
    int output = 2;
    if (measure.value > (setpoint.value + deadband/2) )     {output = 1;}
    else if (measure.value < (setpoint.value - deadband/2)) {output = 3;}
    else {output = 2;}
    return (output);
}



