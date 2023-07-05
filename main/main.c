/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_flash.h"
#include "mdns_service.h"
#include <string.h>
#include "wifi.h"
#include "defines.h"
#include "nvs_flash.h"
#include "batsignal_server.h"
#include "lamp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "batsignal_client.h"

#define TAG "main"

#define PIN_SWITCH 21

// xQueueHandle interruptQueue;
QueueHandle_t interruptQueue;

static void IRAM_ATTR gpio_isr_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
}

void buttonPushedTask(void *params)
{
    int pinNumber, count = 0;
    while (true)
    {
        if (xQueueReceive(interruptQueue, &pinNumber, portMAX_DELAY))
        {
            // disable the interrupt to debounce
            gpio_isr_handler_remove(pinNumber);

            // wait a while for the button to be released
            do
            {
                vTaskDelay(pdMS_TO_TICKS(20));
            } while (gpio_get_level(PIN_SWITCH) == 1);

            // do our button logic here
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(PIN_SWITCH));
            send_command("ring");
            // enable the interrupt again
            gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH);
        }
    }
}
void init_button()
{
    // gpio_pad_select_gpio(PIN_SWITCH);
    gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
    gpio_pulldown_en(PIN_SWITCH);
    gpio_pullup_dis(PIN_SWITCH);
    gpio_set_intr_type(PIN_SWITCH, GPIO_INTR_POSEDGE);

    interruptQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH);
}

void init_batsignal()
{
    init_led();
    set_color(0, 0, 0, 0);

    // initialize batsignal server
    initialise_batsignal_server();
}

void init_batbutton()
{
    init_button();
}

void app_main(void)
{
    // initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // connect to wifi
    initialise_wifi();

    // initialize mdns service
    initialise_mdns();

#ifdef BATSIGNAL
    init_batsignal();
#endif

#ifdef BATBUTTON
    init_batbutton();
#endif
}
