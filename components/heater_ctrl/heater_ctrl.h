/* esp_event (event loop library) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef HEATER_CTRL_H_
#define HEATER_CTRL_H_

#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_timer.h"
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declarations for the event source
#define TASK_ITERATIONS_COUNT        10      // number of times the task iterates
#define TASK_PERIOD                2000      // period of the task loop in milliseconds

ESP_EVENT_DECLARE_BASE(HEATER_EVENTS);         // declaration of the task events family

// Heater status 
enum{
    NOT_OPERATIVE,
    OPERATIVE_WORKING,
    OPERATIVE_WAITING
} heater_status;

// Actions are based on objects and objects data. Events only inform to check data and act based on that.
// Events register may be made based only on BASE, without event_id. No object data is required as it will be read directly form objects.
enum {
    HEATER_EVENT_SETTING_UPDATE,            
    HEATER_EVENT_PROGRAM_UPDATE,            
    HEATER_EVENT_TEMP_UPDATE,               // Evaluate temperature and command heater relay. Plan for multi-thermostat option (only one controls heater)
    HEATER_EVENT_TIME_UPDATE,               
    HEATER_EVENT_HEATER_UPDATE              
} heater_events;

// heater_ctrl_loop parameters structure: Used to pass parameters to initialize loop
typedef struct {
	uint32_t 	            ulLoopPeriod; 		    /* loop period in ms. */
	esp_event_loop_handle_t event_loop_handle; 		/* event loop handler where events will be registered by heater_ctrl_loop */
    TaskHandle_t *          pxTaskHandle;           /* heater task handle */
	} heater_ctrl_loop_params_t2;

typedef struct {
	uint32_t 	            ulLoopPeriod; 		    /* loop period in ms. */
	esp_event_loop_handle_t event_loop_handle; 		/* event loop handler where events will be registered by heater_ctrl_loop */
    TaskHandle_t *          pxTaskHandle;           /* heater task handle */
    measure_t *             pxtemperature;
	} heater_ctrl_loop_params_t;


void heater_ctrl_loop(void *pvParameter);
void heater_test_loop(void *pvParameter);
void heater_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);


#ifdef __cplusplus
}
#endif

#endif // #ifndef HEATER_CTRL_H_



