#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "lamp.h"
#include "defines.h"

#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3

const char *TAGG = "socket server";

static void handle_command(const char *command)
{
    ESP_LOGI(TAGG, "command '%s'", command);
    if (strcmp(command, "ring") == 0)
    {
        ring();
    }
    /* more else if clauses */
    else /* default: */
    {
        ESP_LOGI(TAGG, "unknown command '%s'", command);
    }
}
static void handle_connection(const int sock)
{
    int len;
    char rx_buffer[128];

    do
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0)
        {
            ESP_LOGE(TAGG, "error occurred during receiving: errno %d", errno);
            break;
        }

        if (len == 0)
        {
            ESP_LOGW(TAGG, "connection closed");
            break;
        }

        rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
        ESP_LOGI(TAGG, "received %d bytes: %s", len, rx_buffer);
        handle_command(rx_buffer);

        // send() can return less bytes than supplied length.
        // Walk-around for robust implementation.
        int to_write = len;
        while (to_write > 0)
        {
            int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
            if (written < 0)
            {
                ESP_LOGE(TAGG, "Error occurred during sending: errno %d", errno);
            }
            to_write -= written;
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(SERVER_PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAGG, "unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAGG, "socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAGG, "socket unable to bind: errno %d", errno);
        ESP_LOGE(TAGG, "IPPROTO: %d", addr_family);
        goto cleanup;
    }

    ESP_LOGI(TAGG, "socket bound, port %d", SERVER_PORT);

    err = listen(listen_sock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAGG, "error occurred during listen: errno %d", errno);
        goto cleanup;
    }

    while (1)
    {
        ESP_LOGI(TAGG, "socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            ESP_LOGE(TAGG, "unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        // Convert ip address to string
        if (source_addr.ss_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAGG, "ocket accepted ip address: %s", addr_str);

        handle_connection(sock);

        shutdown(sock, 0);
        close(sock);
    }

cleanup:
    close(listen_sock);
    vTaskDelete(NULL);
}

void initialise_batsignal_server()
{
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
}