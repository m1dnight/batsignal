#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#include "esp32/rom/uart.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"

#include "batcommand.h"

#define TAG "wifi"

/// @brief converts a mac address to a string.
char *mac_to_str(char *buffer, uint8_t *mac)
{
    sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buffer;
}

/// @brief returns the mac address of this device in a buffer.
void get_current_mac(uint8_t *mac) { esp_efuse_mac_get_default(mac); }

/// @brief callback when a message was sent.
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

/// @brief callback when a message was received.
void on_receive(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    uint8_t *mac_addr = esp_now_info->src_addr;
    char     buffer[13];
    ESP_LOGI(TAG, "got message from %s", mac_to_str(buffer, (uint8_t *)mac_addr));

    printf("message: %.*s\n", data_len, data);
    handle_command((char *)data);
}

void add_peer(uint8_t *mac)
{
    printf("adding peer: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    fflush(stdout);
    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    memcpy(peer.peer_addr, mac, 6);

    esp_now_add_peer(&peer);
}

void send_message(const char *message)
{
    char buffer[250];
    sprintf(buffer, message);
    ESP_ERROR_CHECK(esp_now_send(NULL, (uint8_t *)buffer, strlen(buffer)));
}

void initialise_wifi(void)
{
    // init the wifi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // after the wifi, init esp now
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_sent));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_receive));
}

void wake_up_wifi()
{
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_now_init());
}

void sleep_wifi()
{
    ESP_ERROR_CHECK(esp_now_deinit());
    ESP_ERROR_CHECK(esp_wifi_stop());
    uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
}
