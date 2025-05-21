#include "wifi_controller.h"

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_netif.h"

static const char* TAG = "wifi_controller";

static SemaphoreHandle_t wifi_mutex = NULL;
static bool wifi_initialized = false;
static esp_event_handler_instance_t wifi_sta_event_instance = NULL;


// Helper function to log MAC address as string "xx:xx:xx:xx:xx:xx"
void mac_address_to_str(const uint8_t* mac, char* str, size_t str_len) {
    if (!mac || !str || str_len < 18) { // 17 chars + null terminator
        if (str && str_len > 0) str[0] = '\0';
        return;
    }
    snprintf(str, str_len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base != WIFI_EVENT) {
        return;
    }

    switch (event_id) {
        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WiFi AP started");
            break;
        case WIFI_EVENT_AP_STACONNECTED: {
            wifi_event_ap_staconnected_t* evt = (wifi_event_ap_staconnected_t*) event_data;
            mac_address_to_str(evt->mac, (char*)event_data, sizeof(wifi_event_ap_staconnected_t));
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED: {
            wifi_event_ap_stadisconnected_t* evt = (wifi_event_ap_stadisconnected_t*) event_data;
            mac_address_to_str(evt->mac, (char*)event_data, sizeof(wifi_event_ap_stadisconnected_t));
            break;
        }
        default:
            ESP_LOGD(TAG, "Event ID: %d", (int)event_id);
            break;
    }
}

esp_err_t wifi_controller_ap_init(const char* ssid, const char* password,
                                 uint8_t channel, uint8_t max_conn, bool ssid_hidden)
{
    if (!ssid || strlen(ssid) == 0 || strlen(ssid) > 32) {
        ESP_LOGE(TAG, "Invalid SSID");
        return ESP_ERR_INVALID_ARG;
    }

    if (!wifi_mutex) {
        wifi_mutex = xSemaphoreCreateMutex();
        if (!wifi_mutex) {
            ESP_LOGE(TAG, "Failed to create mutex");
            return ESP_FAIL;
        }
    }

    if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take wifi mutex");
        return ESP_ERR_TIMEOUT;
    }

    if (wifi_initialized) {
        ESP_LOGW(TAG, "WiFi already initialized");
        xSemaphoreGive(wifi_mutex);
        return ESP_OK;
    }

    if (password && strlen(password) > 0 && strlen(password) < 8) {
        ESP_LOGE(TAG, "Password too short for WPA2 (minimum 8 characters)");
        xSemaphoreGive(wifi_mutex);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS flash init failed: %s", esp_err_to_name(err));
        xSemaphoreGive(wifi_mutex);
        return err;
    }

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        xSemaphoreGive(wifi_mutex);
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        xSemaphoreGive(wifi_mutex);
        return err;
    }

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        xSemaphoreGive(wifi_mutex);
        return err;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &wifi_sta_event_instance);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(err));
        esp_wifi_deinit();
        xSemaphoreGive(wifi_mutex);
        return err;
    }

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = {0},
            .ssid_len = 0,
            .channel = channel,
            .password = {0},
            .max_connection = max_conn,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = ssid_hidden ? 1 : 0,
        }
    };

    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(ssid);

    if (password && strlen(password) > 0) {
        strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        goto cleanup;
    }

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        goto cleanup;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        goto cleanup;
    }

    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi AP started. SSID: '%s', Channel: %d, Max connections: %d, Hidden: %d",
             ssid, channel, max_conn, ssid_hidden);

    xSemaphoreGive(wifi_mutex);
    return ESP_OK;

cleanup:
    if (wifi_sta_event_instance) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_sta_event_instance);
        wifi_sta_event_instance = NULL;
    }
    esp_wifi_deinit();
    xSemaphoreGive(wifi_mutex);
    return err;
}

esp_err_t wifi_controller_ap_deinit(void)
{
    if (!wifi_mutex) {
        ESP_LOGE(TAG, "WiFi mutex not created");
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(wifi_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take wifi mutex");
        return ESP_ERR_TIMEOUT;
    }

    if (!wifi_initialized) {
        ESP_LOGW(TAG, "WiFi not initialized");
        xSemaphoreGive(wifi_mutex);
        return ESP_OK;
    }

    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop failed: %s", esp_err_to_name(err));
    }

    if (wifi_sta_event_instance) {
        err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_sta_event_instance);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to unregister event handler: %s", esp_err_to_name(err));
        }
        wifi_sta_event_instance = NULL;
    }

    err = esp_wifi_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_deinit failed: %s", esp_err_to_name(err));
    }

    wifi_initialized = false;
    ESP_LOGI(TAG, "WiFi AP deinitialized");

    xSemaphoreGive(wifi_mutex);
    return err;
}

esp_err_t wifi_controller_sta_init(void)
{
    if (wifi_initialized) {
        ESP_LOGW(TAG, "Wi-Fi STA already initialized.");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Wi-Fi STA...");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS flash init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &wifi_sta_event_instance);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(err));
        esp_wifi_deinit();
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        goto cleanup;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        goto cleanup;
    }

    wifi_initialized = true;
    ESP_LOGI(TAG, "Wi-Fi STA successfully initialized.");

    return ESP_OK;

cleanup:
    if (wifi_sta_event_instance) {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_sta_event_instance);
        wifi_sta_event_instance = NULL;
    }
    esp_wifi_deinit();
    return err;
}

esp_err_t wifi_controller_sta_deinit(void)
{
    if (!wifi_initialized) {
        ESP_LOGW(TAG, "Wi-Fi STA not initialized.");
        return ESP_OK;
    }

    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop failed: %s", esp_err_to_name(err));
    }

    if (wifi_sta_event_instance) {
        err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_sta_event_instance);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to unregister event handler: %s", esp_err_to_name(err));
        }
        wifi_sta_event_instance = NULL;
    }

    err = esp_wifi_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_deinit failed: %s", esp_err_to_name(err));
    }

    wifi_initialized = false;
    ESP_LOGI(TAG, "Wi-Fi STA deinitialized.");

    return err;
}