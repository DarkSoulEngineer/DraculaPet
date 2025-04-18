#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <stdbool.h>

extern bool wifi_initialized;

/*
 * @brief Initialize WiFi station mode
**/
bool wifi_controller_sta_init();

#endif
