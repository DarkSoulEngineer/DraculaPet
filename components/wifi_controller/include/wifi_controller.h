#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Convert MAC address bytes to string format "xx:xx:xx:xx:xx:xx"
 * 
 * @param mac      Pointer to 6-byte MAC address array
 * @param str      Output buffer to hold MAC string (must be at least 18 bytes)
 * @param str_len  Length of the output buffer
 */
void mac_address_to_str(const uint8_t* mac, char* str, size_t str_len);

/**
 * @brief Initialize and start WiFi Access Point
 * 
 * @param ssid        Null-terminated SSID string (max 32 chars)
 * @param password    Null-terminated password string (min 8 chars if WPA2, or empty for open)
 * @param channel     WiFi channel number (1-13)
 * @param max_conn    Maximum simultaneous connections (1-10)
 * @param ssid_hidden Whether the SSID is hidden (true/false)
 * @return esp_err_t  ESP_OK on success, error code otherwise
 */
esp_err_t wifi_controller_ap_init(const char* ssid, const char* password, 
                                 uint8_t channel, uint8_t max_conn, bool ssid_hidden);

/**
 * @brief Stop and deinitialize WiFi Access Point
 * 
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t wifi_controller_ap_deinit(void);

/**
 * @brief Initialize and start WiFi Station Mode
 * 
 * @return esp_err_t  ESP_OK on success, error code otherwise
 */
esp_err_t wifi_controller_sta_init();

/**
 * @brief Deinitialize and start WiFi Station Mode
 * 
 * @return esp_err_t  ESP_OK on success, error code otherwise
 */
esp_err_t wifi_controller_sta_deinit(void);

#endif // WIFI_CONTROLLER_H
