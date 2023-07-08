#include "defines.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip_encoder.h"
#include <string.h>

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000
#define EXAMPLE_CHASE_SPEED_MS      10

#define TAG "Led Light"

#define LED_PIN   22
#define LED_COUNT 12

static uint8_t pxls[LED_COUNT * 4];

rmt_channel_handle_t  led_chan = NULL;
rmt_transmit_config_t tx_config;
rmt_encoder_handle_t  led_encoder = NULL;

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

void set_color(uint32_t red, uint32_t green, uint32_t blue, uint32_t white)
{
    // Put all leds on the given RGB values.
    for (int i = 0; i < 4; i++) {
        for (int j = i; j < LED_COUNT; j += 4) {
            pxls[j * 4 + 0] = green; // groen
            pxls[j * 4 + 1] = red;   // rood
            pxls[j * 4 + 2] = blue;  // groen?
            pxls[j * 4 + 3] = white; // wit
        }
        // Flush RGB values to LEDs
    }
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pxls, sizeof(pxls), &tx_config));
}

void rainbow()
{
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;

    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pxls, sizeof(pxls), &tx_config));
    vTaskDelay(pdMS_TO_TICKS(4000));

    while (1) {
        for (int i = 0; i < 4; i++) {
            for (int j = i; j < LED_COUNT; j += 4) {
                // Build RGB pixels
                hue = j * 360 / LED_COUNT + start_rgb;
                led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
                pxls[j * 4 + 0] = red;   // groen
                pxls[j * 4 + 1] = green; // rood
                pxls[j * 4 + 2] = blue;  // groen?
                pxls[j * 4 + 3] = 255;   // wit
            }
            // Flush RGB values to LEDs
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pxls, sizeof(pxls), &tx_config));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            memset(pxls, 0, sizeof(pxls));
            ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pxls, sizeof(pxls), &tx_config));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        }
        start_rgb += 60;
    }
}

void ring()
{
    uint32_t r = 255, g = 0, b = 0, w = 0;

    int pause = 200;
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, pxls, sizeof(pxls), &tx_config));

    int flickers = 10;
    while (flickers > 0) {
        set_color(r, g, b, w);
        vTaskDelay(pdMS_TO_TICKS(pause));
        set_color(0, 0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(pause));
        flickers--;
    }
}

void init_led()
{
    ESP_LOGI(TAG, "Initializing LED");

    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = LED_PIN,
        .mem_block_symbols = 64,        // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4,         // set the number of transactions that can be
                                        // pending in the background
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led encoder");

    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };

    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    ESP_LOGI(TAG, "Start LED rainbow chase");
    tx_config = (rmt_transmit_config_t){
        .loop_count = 0, // no transfer loop
    };

    memset(pxls, 0, sizeof(pxls));

    set_color(0, 0, 0, 0);
}
