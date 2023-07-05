#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "defines.h"

const char *TAG = "socket client";

void send_command(char *command)
{
    char rx_buffer[128];
    char host_ip[] = "192.168.4.57";
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "socket created, connecting to %s:%d", host_ip, SERVER_PORT);

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "socket unable to connect: errno %d", errno);
        return;
    }

    ESP_LOGI(TAG, "successfully connected");

    err = send(sock, command, strlen(command), 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "error occurred during sending: errno %d", errno);
        return;
    }

    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    // error occurred during receiving
    if (len < 0)
    {
        ESP_LOGE(TAG, "recv failed: errno %d", errno);
        return;
    }
    // data received
    else
    {
        rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
        ESP_LOGI(TAG, "received %d bytes from %s:", len, host_ip);
        ESP_LOGI(TAG, "%s", rx_buffer);
    }

    if (sock != -1)
    {
        ESP_LOGE(TAG, "closing socket");
        shutdown(sock, 0);
        close(sock);
    }
}