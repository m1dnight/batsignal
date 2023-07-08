/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "batsignal_client.h"
#include "batsignal_server.h"
#include "button.h"
#include "defines.h"
#include "lamp.h"
#include "mdns_service.h"
#include "wifi.h"

#include "driver/rtc_io.h"
#include "esp32/rom/uart.h"
#include "esp_timer.h"

// button sleep

#define TAG "main"

void init_batsignal()
{
    init_led();

    // initialize batsignal server
    initialise_batsignal_server();
}

void init_batbutton() { init_button(); }

RTC_DATA_ATTR int timesWokenUp = 0;

void app_main(void)
{

    // initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // connect to wifi
    initialise_wifi();

    // initialize mdns service
    initialise_mdns();

#ifdef BATSIGNAL
    ESP_LOGI(TAG, "batsignal");
    init_batsignal();
#endif

    // #ifdef BATBUTTON
    //     ESP_LOGI(TAG, "batbutton");
    //     init_batbutton();
    // #endif

    // not needed for idf v5
    // gpio_pad_select_gpio(PIN_SWITCH);
    gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
    gpio_pulldown_dis(PIN_SWITCH);
    gpio_pullup_en(PIN_SWITCH);
    gpio_wakeup_enable(PIN_SWITCH, GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(5000000);

    while (true) {
        // if the button is pressed, wait for it to be released before going on.
        if (rtc_gpio_get_level(PIN_SWITCH) == 0) {
            printf("please release button\n");
            do {
                vTaskDelay(pdMS_TO_TICKS(10));
            } while (rtc_gpio_get_level(PIN_SWITCH) == 0);
        }

        printf("going for a nap\n");
        uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

        int64_t before = esp_timer_get_time();

        esp_light_sleep_start();

        // when waking up, do the following.
        send_command("ring");

        int64_t after = esp_timer_get_time();

        esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

        printf("napped for %lld, reason was %s\n", (after - before) / 1000, reason == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "button");
    }
}
