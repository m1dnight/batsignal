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

void init_batsignal()
{
    init_led();

    // initialize batsignal server
    initialise_batsignal_server();
}

void init_batbutton() { init_button(); }

uint8_t mac_btn[6] = {0x0c, 0xdc, 0x7e, 0xcb, 0x22, 0xb0};
uint8_t mac_lmp[6] = {0x54, 0x43, 0xb2, 0x51, 0xd0, 0xe8};

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

    if (is_lamp()) {
        add_peer(mac_btn);
    }
    else {
        add_peer(mac_lmp);
    }
    if (!lamp) {

        char send_buffer[250];

        sprintf(send_buffer, "Hello from %s message %d", mac_str, 123);
        ESP_ERROR_CHECK(esp_now_send(NULL, (uint8_t *)send_buffer, strlen(send_buffer)));
        vTaskDelay(pdMS_TO_TICKS(1000));

        esp_sleep_enable_timer_wakeup(5 * 1000000);

        while (1) {
            printf("going for a nap\n");

            ESP_ERROR_CHECK(esp_now_deinit());
            ESP_ERROR_CHECK(esp_wifi_stop());
            uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

            esp_light_sleep_start();

            printf("woke up\n");
            ESP_ERROR_CHECK(esp_wifi_start());

            ESP_ERROR_CHECK(esp_now_init());

            esp_now_peer_info_t peer;
            memset(&peer, 0, sizeof(esp_now_peer_info_t));
            memcpy(peer.peer_addr, peer_mac, 6);

            esp_now_add_peer(&peer);

            char send_buffer[250];

            sprintf(send_buffer, "Hello from %s message %d", mac_str, 123);
            ESP_ERROR_CHECK(esp_now_send(NULL, (uint8_t *)send_buffer, strlen(send_buffer)));
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    //     // connect to wifi
    //     initialise_wifi();

    //     // initialize mdns service
    //     initialise_mdns();

    // #ifdef BATSIGNAL
    //     ESP_LOGI(TAG, "batsignal");
    //     init_batsignal();
    // #endif

    //     // #ifdef BATBUTTON
    //     //     ESP_LOGI(TAG, "batbutton");
    //     //     init_batbutton();
    //     // #endif

    // esp_sleep_enable_timer_wakeup(500000000);
    // esp_light_sleep_start();

    //     while (true) {
    //         // if the button is pressed, wait for it to be released before going on.
    //         if (rtc_gpio_get_level(PIN_SWITCH) == 0) {
    //             printf("please release button\n");
    //             do {
    //                 vTaskDelay(pdMS_TO_TICKS(10));
    //             } while (rtc_gpio_get_level(PIN_SWITCH) == 0);
    //         }

    //         esp_wifi_stop();
    //         printf("going for a nap\n");
    //         uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

    //         int64_t before = esp_timer_get_time();

    //         esp_light_sleep_start();

    //         // connect to wifi
    //         // initialise_wifi();
    //         esp_wifi_start();
    //         esp_wifi_connect();

    //         // initialize mdns service
    //         initialise_mdns();

    //         // when waking up, do the following.
    //         send_command("ring");

    //         int64_t after = esp_timer_get_time();

    //         esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

    //         printf("napped for %lld, reason was %s\n", (after - before) / 1000, reason == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "button");
    //     }
}
