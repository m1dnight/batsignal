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

// button sleep

#define TAG "main"

void init_batsignal()
{
    init_led();

    // initialize batsignal server
    initialise_batsignal_server();
}

void init_batbutton() { init_button(); }

RTC_DATA_ATTR static int boot_count = 0;

void app_main(void)
{
    // gpio_pad_select_gpio(PIN_SWITCH);
    rtc_gpio_pulldown_en(PIN_SWITCH);
    rtc_gpio_pullup_dis(PIN_SWITCH);

    // enable wakeup with button
    esp_sleep_enable_ext0_wakeup(PIN_SWITCH, 0);
    printf("going to sleep again");
    fflush(stdout);
    esp_deep_sleep_start();

    //     // initialize storage
    //     esp_err_t ret = nvs_flash_init();
    //     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //         ESP_ERROR_CHECK(nvs_flash_erase());
    //         ESP_ERROR_CHECK(nvs_flash_init());
    //     }

    //     // connect to wifi
    //     initialise_wifi();

    //     // initialize mdns service
    //     initialise_mdns();

    // #ifdef BATSIGNAL
    //     ESP_LOGI(TAG, "batsignal");
    //     init_batsignal();
    // #endif

    // #ifdef BATBUTTON
    //     ESP_LOGI(TAG, "batbutton");
    //     init_batbutton();
    // #endif

    //     esp_deep_sleep_start();
}
