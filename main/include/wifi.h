#ifndef __wifi
#define __wifi

#include "esp_event.h"

#define WIFI_SSID     "IoT"
#define WIFI_PASS     "2cool4school"
#define MAXIMUM_RETRY 3

char *mac_to_str(char *buffer, uint8_t *mac);

void initialise_wifi(void);

void sleep_wifi(void);

void wake_up_wifi(void);

void get_current_mac(uint8_t *mac);

void add_peer(uint8_t *mac);

void send_message(const char *message);

#endif