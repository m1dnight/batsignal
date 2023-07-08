#include "esp_log.h"
#include <string.h>

#include "defines.h"
#include "lamp.h"

#define KEEPALIVE_IDLE     5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT    3

const char *TAGG = "socket server";

static void handle_command(const char *command)
{
    ESP_LOGI(TAGG, "command '%s'", command);
    if (strcmp(command, "ring") == 0) {
        ring();
    }
    else {
        ESP_LOGI(TAGG, "unknown command '%s'", command);
    }
}