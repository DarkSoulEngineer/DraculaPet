#include "wifi_controller.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ap_scanner.h"


#define TAG "wifi_controller"

bool wifi_init = false; // Flag to track WiFi initialization status


static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {                                   // Check if the event is from the WiFi subsystem
        switch (event_id) {
            case WIFI_EVENT_SCAN_DONE:                                // If the scan is done
                ESP_LOGI(TAG, "Wi-Fi scan completed successfully!");  break;
            case WIFI_EVENT_STA_START:                                // If the station starts
                ESP_LOGI(TAG, "Station started.");
                esp_wifi_connect();                                   break;
            case WIFI_EVENT_STA_CONNECTED:                            // If the station connects to an AP
                ESP_LOGI(TAG, "Station connected to AP.");            break;
            case WIFI_EVENT_STA_DISCONNECTED:                         // If the station disconnects from an AP
                ESP_LOGI(TAG, "Station disconnected from AP.");
                esp_wifi_connect();                                   break;
            default:                                                  // Default
                ESP_LOGI(TAG, "Unhandled WiFi event: %ld", event_id); break;
        }
    }
}

bool wifi_controller_sta_init() {
    if (wifi_init) {                                                  // Check if WiFi is already initialized
        ESP_LOGW("WiFi", "Wi-Fi already initialized.");   
        return false;     
    }     

    ESP_LOGI(TAG, "Initializing Wi-Fi...");   
    ESP_ERROR_CHECK(nvs_flash_init());                                // Initialize NVS (required for WiFi)
    ESP_ERROR_CHECK(esp_netif_init());                                // Initialize network interface
    ESP_ERROR_CHECK(esp_event_loop_create_default());                 // Create default event loop
    esp_netif_create_default_wifi_sta();                              // Create default Wi-Fi station interface

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();              // Initialize Wi-Fi configuration
    esp_wifi_init(&cfg);                                              // Initialize Wi-Fi

    esp_event_handler_instance_t instance;                            // Register event handler
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &instance);
    
    esp_wifi_set_mode(WIFI_MODE_STA);                                 // Set Wi-Fi mode to station
    esp_wifi_start();                                                 // Start Wi-Fi

    wifi_init = true;                                                 // Mark as initialized
    ESP_LOGI("WiFi", "Wi-Fi successfully initialized.");
    return true;
}