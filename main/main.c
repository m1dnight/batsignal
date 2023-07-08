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
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// button sleep

#define TAG "main"

void init_batsignal() { init_led(); }

void init_batbutton() { init_button(); }

uint8_t mac_btn[6] = {0x0c, 0xdc, 0x7e, 0xcb, 0x22, 0xb0};
uint8_t mac_lmp[6] = {0x54, 0x43, 0xb2, 0x51, 0xd0, 0xe8};

/// @brief returns true if this device is a lamp.
bool is_lamp()
{
    uint8_t this_mac[6];
    get_current_mac(this_mac);
    char mac_str[13];
    mac_to_str(mac_str, this_mac);

    if (memcmp(this_mac, mac_btn, 6) == 0) {
        ESP_LOGI(TAG, "this is the button");
        return false;
    }
    else if (memcmp(this_mac, mac_lmp, 6) == 0) {
        ESP_LOGI(TAG, "this is the lamp");
        return true;
    }
    ESP_LOGE(TAG, "this not a lamp or a button");
    return false;
}

void app_main(void)
{

    // initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    bool lamp = is_lamp();

    // initialize wifi
    initialise_wifi();

    if (lamp) {
        init_batsignal();
    }
    else {
        init_batbutton();
    }

    // // the button should power on and then go to sleep.
    // // when the button is pressed the button should wake up
    // // and send a message to the lamp.
    // if (!lamp) {
    //     esp_sleep_enable_timer_wakeup(5 * 1000000);
    //     while (1) {
    //         printf("woke up from sleep\n");
    //         // wake up the wifi
    //         wake_up_wifi();

    //         // readd all the peers after sleep
    //         add_peer(mac_lmp);

    //         send_message("ring");

    //         printf("going back to sleep\n");

    //         sleep_wifi();

    //         esp_deep_sleep_start();
    //     }
    // }
}
