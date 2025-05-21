#include "esp_err.h"         // For esp_err_t, ESP_OK, esp_err_to_name()
#include "esp_log.h"         // For ESP_LOGE, ESP_LOGI, etc.

#include "main.h"


// C Modules
extern "C" {
    #include "wifi_controller.h"
}

static const char* TAG = "main";

void app_main() {
    esp_err_t err = wifi_controller_ap_init("SUPERSECRETAP", "SUPERSECRETPASSWORD", 6, 4, true);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Failed to init WiFi AP: %s", esp_err_to_name(err));
    }
}
