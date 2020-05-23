/* esp_event (event loop library) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef MOD_GPIO_H_
#define MOD_GPIO_H_


#include <esp_log.h>


#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_OUTPUT_01    18
#define GPIO_OUTPUT_02    19
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_01) | (1ULL<<GPIO_OUTPUT_02))

#define GPIO_INPUT_01     4
#define GPIO_INPUT_02     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_01) | (1ULL<<GPIO_INPUT_02))
#define ESP_INTR_FLAG_DEFAULT 0

#define ON  1
#define OFF 0


/****************************************************************************** 
* gpio_task_create
*******************************************************************************
 * @brief gpio_task_create: Creates gpio_task (listener) and gpio_isr_handler to process GPIO input events
*******************************************************************************/
int gpio_task_create(void);


/****************************************************************************** 
* gpio_task_destroy
*******************************************************************************
* @brief gpio_task_destroy: Destroys gpio task listener
*******************************************************************************/
void gpio_task_destroy(void);



#ifdef __cplusplus
}
#endif

#endif // #ifndef BMP280_CTRL_LOOP_H_