#include "driver/rtc_io.h"
#include "esp_sleep.h"

#include "defines.h"

void init_button()
{
    // button pin 13 is connected to the ground
    // => pullup
    rtc_gpio_pulldown_dis(PIN_SWITCH);
    rtc_gpio_pullup_en(PIN_SWITCH);
    esp_sleep_enable_ext0_wakeup(PIN_SWITCH, 0);
}
