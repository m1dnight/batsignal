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