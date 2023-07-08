#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "batsignal_client.h"
#include "defines.h"

// xQueueHandle interruptQueue;
QueueHandle_t interruptQueue;

static void IRAM_ATTR gpio_isr_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
}

void buttonPushedTask(void *params)
{
    int pinNumber, count = 0;
    while (true) {
        if (xQueueReceive(interruptQueue, &pinNumber, portMAX_DELAY)) {
            // disable the interrupt to debounce
            gpio_isr_handler_remove(pinNumber);

            // wait a while for the button to be released
            do {
                vTaskDelay(pdMS_TO_TICKS(20));
            } while (gpio_get_level(PIN_SWITCH) == 1);

            // do our button logic here
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(PIN_SWITCH));
            send_command("ring");
            // enable the interrupt again
            gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH);
        }
    }
}

// void init_button()
// {
//     // gpio_pad_select_gpio(PIN_SWITCH);
//     gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);
//     gpio_pulldown_en(PIN_SWITCH);
//     gpio_pullup_dis(PIN_SWITCH);
//     gpio_set_intr_type(PIN_SWITCH, GPIO_INTR_POSEDGE);

//     interruptQueue = xQueueCreate(10, sizeof(int));
//     xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

//     gpio_install_isr_service(0);
//     gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH);
// }

void init_button()
{
    // gpio_pad_select_gpio(PIN_SWITCH);
    rtc_gpio_pulldown_en(PIN_SWITCH);
    rtc_gpio_pullup_dis(PIN_SWITCH);

    // enable wakeup with button
    esp_sleep_enable_ext0_wakeup(PIN_SWITCH, 0);

    rtc_gpio_set_direction(PIN_SWITCH, GPIO_MODE_INPUT);

    printf("going to sleep again");
    fflush(stdout);
    esp_deep_sleep_start();

    // interruptQueue = xQueueCreate(10, sizeof(int));
    // xTaskCreate(buttonPushedTask, "buttonPushedTask", 2048, NULL, 1, NULL);

    // gpio_install_isr_service(0);
    // gpio_isr_handler_add(PIN_SWITCH, gpio_isr_handler, (void *)PIN_SWITCH);
}