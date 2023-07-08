
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

// own code
#include "lamp.h"

#define TAG "batcommand"

void handle_command(const char *command)
{
    ESP_LOGI(TAG, "command '%s'", command);
    if (strcmp(command, "ring") == 0) {
        ring();
    }
    /* more else if clauses */
    else /* default: */
    {
        ESP_LOGI(TAG, "unknown command '%s'", command);
    }
}