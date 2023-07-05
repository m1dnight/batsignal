#ifndef __wifi
#define __wifi

#include "esp_event.h"

#define WIFI_SSID     "IoT"
#define WIFI_PASS     "2cool4school"
#define MAXIMUM_RETRY 3

void initialise_wifi(void);

#endif