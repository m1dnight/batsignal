#ifndef __defines
#define __defines

// service parameters (available on both)
#define SERVER_PORT 8080

// parameters for the button wiring
#define PIN_SWITCH 13

// led ring
#define LED_PIN   22
#define LED_COUNT 12

// #define BATSIGNAL
#define BATBUTTON

// client parameters
#ifdef BATSIGNAL
#define WIFI_SSID "IoT"
#define WIFI_PASS "2cool4school"

#define HOSTNAME "batsignal"
#define INSTANCE "batsignal"
#define SERVICE  "batsignal"
#endif

// server parameters
#ifdef BATBUTTON
#define WIFI_SSID "IoT"
#define WIFI_PASS "2cool4school"

#define HOSTNAME "batbutton"
#define INSTANCE "batbutton"
#define SERVICE  "batbutton"
#endif

#endif