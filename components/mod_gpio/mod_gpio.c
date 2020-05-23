/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "mod_gpio.h"


static const char* TAG = "MOD_GPIO";
static xQueueHandle gpio_evt_queue = NULL;      //queue to send input "events"
TaskHandle_t  TaskHandle_mod_gpio;           // GPIO task handle
TaskHandle_t* pxTaskHandle_mod_gpio;           // GPIO task handle



/****************************************************************************** 
* gpio_isr_handler
*******************************************************************************
* @brief gpio_isr_handler: receives input GPIO signals and sends to queue to be processed.
*******************************************************************************/

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}


/****************************************************************************** 
* gpio_task
*******************************************************************************
* @brief gpio_task: process GPIO input events received via gpio queue
*******************************************************************************/
static void gpio_task(void* arg) {
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));

            switch (io_num) {
                case GPIO_OUTPUT_01:
                    ESP_LOGI(TAG, "GPIO[%d] (BUTTON 01) received, level: %d\n", io_num, gpio_get_level(io_num));
                    break;

                case GPIO_OUTPUT_02:
                    ESP_LOGI(TAG, "GPIO[%d] (BUTTON 02) received, level: %d\n", io_num, gpio_get_level(io_num));
                    break;

                default:
                    ESP_LOGI(TAG, "GPIO[%d] (Unknown) received, level: %d\n", io_num, gpio_get_level(io_num));
                    break;
            }
        }
    }
}


/****************************************************************************** 
* gpio_task_create
*******************************************************************************
* @brief gpio_task_create: Creates gpio_isr_handler and gpio_task (listener) to process GPIO input events
* @return -> 0: OK / 1: FAIL   
*******************************************************************************/
int gpio_task_create(void) {
    int error = 0;
    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;                    //set as output mode
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;          //disable interrupt
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;         //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pull_down_en = 0;                           //disable pull-down mode
    io_conf.pull_up_en = 0;                             //disable pull-up mode
    gpio_config(&io_conf);                              //configure GPIO with the given settings

    io_conf.mode = GPIO_MODE_INPUT;                     //set as input mode    
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;          //interrupt of rising edge
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;          //bit mask of the pins, use GPIO4/5 here
    io_conf.pull_up_en = 1;                             //enable pull-up mode
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));                                    //create a queue to handle gpio event from isr
    //xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);                            //start gpio task

    pxTaskHandle_mod_gpio = &TaskHandle_mod_gpio;
    if ( xTaskCreatePinnedToCore(gpio_task, "gpio_task", 1024 * 2, NULL, 5,
                                 pxTaskHandle_mod_gpio, APP_CPU_NUM) != pdPASS ) {    
        error = 1;
        ESP_LOGE(TAG, "GPIO Task creation failed");
        } 
    else { 
        error = 0;
        ESP_LOGI(TAG, "GPIO Task created");        
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);                                    //install gpio isr service
        gpio_isr_handler_add(GPIO_INPUT_01, gpio_isr_handler, (void*) GPIO_INPUT_01);       //hook isr handler for specific gpio pin
        gpio_isr_handler_add(GPIO_INPUT_02, gpio_isr_handler, (void*) GPIO_INPUT_02);       //hook isr handler for specific gpio pin
        }
    return (error);
}



/****************************************************************************** 
* gpio_task_destroy
*******************************************************************************
* @brief gpio_task_destroy: Destroys gpio task listener
*******************************************************************************/
void gpio_task_destroy(void) {
    vTaskDelete(TaskHandle_mod_gpio);
}

// Relays Control: 
//    gpio_set_level(GPIO_OUTPUT_02, ON);
//    gpio_set_level(GPIO_OUTPUT_02, OFF);


//    gpio_isr_handler_remove(GPIO_INPUT_01);                                               //remove isr handler for gpio number.
//    gpio_isr_handler_add(GPIO_INPUT_01, gpio_isr_handler, (void*) GPIO_INPUT_01);       //hook isr handler for specific gpio pin again 
//    gpio_set_level(GPIO_OUTPUT_02, ON);



