/* esp_event (event loop library) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef BMP280_CTRL_LOOP_H_
#define BMP280_CTRL_LOOP_H_

#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_timer.h"

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <string.h>
#include <esp_log.h>
#include "bmp280.h"
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declarations for the event source
#define TASK_ITERATIONS_COUNT        10      // number of times the task iterates
#define TASK_PERIOD                2000      // period of the task loop in milliseconds

#define DELAY_1s             (pdMS_TO_TICKS( 1000))
#define DELAY_2s             (pdMS_TO_TICKS( 2000))
#define DELAY_5s             (pdMS_TO_TICKS( 5000))
#define EVENT_MAX_DELAY      (pdMS_TO_TICKS( 5000))

ESP_EVENT_DECLARE_BASE(BMP280_EVENTS);         // declaration of the task events family: BMP280_CONTROL_LOOP (BMP280_CL)

// BMP280 status        //TODO
/*
enum{
    NOT_OPERATIVE,
    OPERATIVE_WORKING,
    OPERATIVE_WAITING
} BMP280_status;  
*/      

// Actions are based on objects and objects data. Events only inform to check data and act based on that.
// Events register may be made based only on BASE, without event_id. No object data is required as it will be read directly form objects.
enum {
    BMP280_CL_CREATE_LOOP,            
    BMP280_CL_KILL_LOOP,            
    BMP280_CL_START,
    BMP280_CL_STOP,
    BMP280_CL_RESET,               
    BMP280_CL_CHANGE_FREQ
} BMP280_CL_events;

typedef struct {
    measure_t temperature;
    measure_t pressure;
    measure_t humidity;
    } BMP280_Measures_t;

// BMP280_ctrl_loop parameters structure: Used to pass parameters to initialize loop
typedef struct {
	uint32_t 	            ulLoopPeriod; 		    /* loop period in ms. */
	esp_event_loop_handle_t event_loop_handle; 		/* event loop handler where events will be registered by BMP280_ctrl_loop */
    TaskHandle_t *          pxTaskHandle;     /* BMP280 task handle */
    int                     sda_gpio;
    int                     scl_gpio;
    int                     I2C_port;
    BMP280_Measures_t *     pxBMP280_Measures;
	} BMP280_control_loop_params_t;


void bmp280_ctrl_loop(void *pvParameter);
void bmp280_test_loop(void *pvParameter);
void bmp280_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);


#ifdef __cplusplus
}
#endif

#endif // #ifndef BMP280_CTRL_LOOP_H_



