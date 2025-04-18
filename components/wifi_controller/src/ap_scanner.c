#include "ap_scanner.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"


#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
static const char* TAG = "wifi_controller/ap_scanner";

static void (*scan_complete_callback)(void) = NULL;                            // Function pointer to callback
static wifictl_ap_records_t ap_records;                                        // static variable to store AP records


void wifictl_scan_nearby_aps() {
    if (!wifi_controller_sta_init()){                                          // Initialize WiFi if not already initialized
        ESP_LOGE(TAG, "Failed to initialize WiFi for scanning.");
        return;
    }

    ESP_LOGD(TAG, "Scanning nearby APs...");
    ap_records.count = CONFIG_SCAN_MAX_AP;                                     // Initialize count to CONFIG_SCAN_MAX_AP

    wifi_scan_config_t scan_config = {
        .ssid = NULL                               ,                           // NULL to scan all SSIDs
        .bssid = NULL                              ,                           // NULL to scan all BSSIDs
        .channel = 0                               ,                           // 0 to scan all channels
        .show_hidden = false                       ,                           // false to hide hidden APs
        .scan_type = WIFI_SCAN_TYPE_ACTIVE         ,                           // Active scan
        .scan_time.active = {.min = 120, .max = 150}                           // Scan time range in milliseconds
    };

    esp_err_t err = esp_wifi_scan_start(&scan_config, true);                   // Start scan
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WiFi scan failed: %s", esp_err_to_name(err));
        return;
    }

    err = esp_wifi_scan_get_ap_records(&ap_records.count, ap_records.records); // Get AP records
    if (err != ESP_OK) {                                                       // Check for errors
        ESP_LOGE(TAG, "Failed to get AP records: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGD(TAG, "Scan done.");

    if (scan_complete_callback != NULL) {                                      // If callback is set, call it
        scan_complete_callback();                                              // Call the callback
    }          
}          
        
const wifictl_ap_records_t *wifictl_get_ap_records() {                         // Getter for AP records
    return &ap_records;                                                        // Return pointer to ap_records
}          
        
const wifi_ap_record_t *wifictl_get_ap_record(unsigned index) {                // Getter for a specific AP record
    if (index >= ap_records.count) {                                           // Check if index is out of bounds
        ESP_LOGE(TAG, "Index out of bounds! %u records available, but %u requested", ap_records.count, index);
        return NULL;
    }
    return &ap_records.records[index];                                         // Return pointer to the AP record at the specified index
}

const char* get_auth_mode_str(wifi_auth_mode_t auth_mode) {                    // Function to convert auth mode to string
    switch (auth_mode) {
        case WIFI_AUTH_OPEN          :return "Open"         ;
        case WIFI_AUTH_WEP           :return "WEP"          ;
        case WIFI_AUTH_WPA_PSK       :return "WPA PSK"      ;   
        case WIFI_AUTH_WPA2_PSK      :return "WPA2 PSK"     ;    
        case WIFI_AUTH_WPA_WPA2_PSK  :return "WPA/WPA2 PSK" ;        
        case WIFI_AUTH_WPA3_PSK      :return "WPA3 PSK"     ;    
        case WIFI_AUTH_WPA2_WPA3_PSK :return "WPA2/WPA3 PSK";         
        case WIFI_AUTH_WAPI_PSK      :return "WAPI PSK"     ;    
        default                      :return "Unknown"      ;   
    }
}

void print_ap_list(const wifictl_ap_records_t *records) {                      // Function to print the list of APs
    if (!records) {                                                            // Check if records are valid
        printf("Failed to get WiFi scan results\n");             
        return;          
    }            

    printf("Total APs found: %d\n", records->count);                           // Print total number of APs found
    for (int i = 0; i < records->count; i++) {                                
        printf("%d. SSID: %s, RSSI: %d dBm, CH: %d, ENC: %s, BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n", 
        i+1                                                                                         ,
        records->records[i].ssid                                                                    ,
        records->records[i].rssi                                                                    ,
        records->records[i].primary                                                                 ,
        get_auth_mode_str(records->records[i].authmode)                                             ,
        records->records[i].bssid[0], records->records[i].bssid[1]                                  , 
        records->records[i].bssid[2], records->records[i].bssid[3]                                  , 
        records->records[i].bssid[4], records->records[i].bssid[5]
        );
    }
    printf("\n");
}