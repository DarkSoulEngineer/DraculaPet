#ifndef AP_SCANNER_H
#define AP_SCANNER_H

#include "esp_wifi.h"
#include "wifi_controller.h"


#ifndef CONFIG_SCAN_MAX_AP                                                  // CONFIG_SCAN_MAX_AP
#define CONFIG_SCAN_MAX_AP 20                                               // Default value is 20
#endif

/** 
 * @brief Converts WiFi auth mode to a readable string.
 * @param authmode WiFi auth mode.
 * @return const char* String representation of the auth mode.
 **/
const char* get_auth_mode_str(wifi_auth_mode_t authmode);

/**
 * @brief Structure to store records of scanned APs.
 * @note This structure contains information about multiple access points.
 **/
typedef struct {
    uint16_t count;                                                        // Number of APs found
    wifi_ap_record_t records[CONFIG_SCAN_MAX_AP];                          // Array of AP records
} wifictl_ap_records_t;

/**
 * @brief Registers a callback to be called when the scan is complete.
 * @param callback Function pointer to the callback function.
 **/
void wifictl_register_scan_callback(void (*callback)(void));

/**
 * @brief Starts a scan for nearby access points.
 * @note This function initiates the scanning process, and the callback will be invoked when it is done.
 **/
void wifictl_scan_nearby_aps(void);

/**
 * @brief Returns the latest scanned AP records.
 * @return Pointer to the structure containing AP records.
 **/
const wifictl_ap_records_t *wifictl_get_ap_records(void);

/**
 * @brief Retrieves a specific AP record by index.
 * @param index Index of the AP record to retrieve.
 * @return Pointer to the AP record at the specified index.
 **/
const wifi_ap_record_t *wifictl_get_ap_record(unsigned index);

/**
 * @brief Prints the list of APs found during the scan.
 * @param records Pointer to the structure containing AP records.
 * @note This function prints the list of APs found during the scan, including their SSID, BSSID, channel, RSSI, and authentication mode.
 **/
void print_ap_list(const wifictl_ap_records_t *records);

#endif // AP_SCANNER_H