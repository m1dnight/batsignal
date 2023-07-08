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

RTC_DATA_ATTR int timesWokenUp = 0;

char *mac_to_str(char *buffer, uint8_t *mac)
{
    sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buffer;
}

// feather 0c:dc:7e:cb:22:b0
// esp     54:43:b2:51:d0:e8

uint8_t mac_btn[6] = {0x0c, 0xdc, 0x7e, 0xcb, 0x22, 0xb0};
uint8_t mac_lmp[6] = {0x54, 0x43, 0xb2, 0x51, 0xd0, 0xe8};

void on_sent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    char buffer[13];
    switch (status) {
    case ESP_NOW_SEND_SUCCESS:
        ESP_LOGI(TAG, "message sent to %s", mac_to_str(buffer, (uint8_t *)mac_addr));
        break;
    case ESP_NOW_SEND_FAIL:
        ESP_LOGE(TAG, "message sent to %s failed", mac_to_str(buffer, (uint8_t *)mac_addr));
        break;
    }
}

void on_receive(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char buffer[13];
    ESP_LOGI(TAG, "got message from %s", mac_to_str(buffer, (uint8_t *)mac_addr));

    printf("message: %.*s\n", data_len, data);
}

void app_main(void)
{

    uint8_t this_mac[6];
    esp_efuse_mac_get_default(this_mac);
    char     mac_str[13];
    uint8_t *peer_mac = mac_btn;
    mac_to_str(mac_str, this_mac);
    ESP_LOGI(TAG, "mac address: '%s'", mac_str);

    bool lamp = false;
    if (memcmp(this_mac, mac_btn, 6) == 0) {
        ESP_LOGI(TAG, "this is the button");
        peer_mac = mac_lmp;
    }
    else if (memcmp(this_mac, mac_lmp, 6) == 0) {
        ESP_LOGI(TAG, "this is the lamp");
        peer_mac = mac_btn;
        lamp = true;
    }
    else {
        ESP_LOGI(TAG, "this is neither the button nor the lamp");
    }

    // initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // init the wifi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_sent));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_receive));

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    memcpy(peer.peer_addr, peer_mac, 6);

    esp_now_add_peer(&peer);

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
