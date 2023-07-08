#ifndef __wifi
#define __wifi

#include "esp_event.h"

#define WIFI_SSID     "IoT"
#define WIFI_PASS     "2cool4school"
#define MAXIMUM_RETRY 3

void initialise_wifi(void);

void deinitalise_wifi(void);

void get_current_mac(uint8_t *mac);

void add_peer(uint8_t *mac);

#endif