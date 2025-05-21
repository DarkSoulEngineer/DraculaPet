#include "esp_err.h"         // For esp_err_t, ESP_OK, esp_err_to_name()
#include "esp_log.h"         // For ESP_LOGE, ESP_LOGI, etc.

#include "main.h"


// C Modules
extern "C" {
    #include "wifi_controller.h"
    #include "ap_scanner.h"
}

static const char* TAG = "main";

void app_main() {
    esp_err_t err = wifi_controller_ap_init("SUPERSECRETAP", "SUPERSECRETPASSWORD", 6, 4, true);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to init WiFi AP: %s", esp_err_to_name(err));
    }
    wifi_controller_ap_deinit();

    err = wifi_controller_sta_init();
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to init WiFi STA: %s", esp_err_to_name(err));
    }
    wifictl_scan_nearby_aps();
    const wifictl_ap_records_t *records = wifictl_get_ap_records();
    print_ap_list(records);
}
