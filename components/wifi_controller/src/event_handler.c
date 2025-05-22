#include "event_handler.h"

#include "esp_wifi_types.h"
#include "esp_netif.h"   // For IP event structs
#include "esp_wifi.h"
#include <inttypes.h>
#include "esp_log.h"
#include "esp_eth.h"
#include <stdio.h>

static const char *TAG = "event_handler";

// Helper function to convert MAC to string "xx:xx:xx:xx:xx:xx"
void mac_address_to_str(const uint8_t* mac, char* str, size_t str_len) {
    if (!mac || !str || str_len < 18) {
        if (str && str_len > 0) str[0] = '\0';
        return;
    }
    snprintf(str, str_len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// New style event handler
void wifi_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    char mac_str[18];

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_WIFI_READY:
                ESP_LOGI(TAG, "WiFi ready");
                break;

            case WIFI_EVENT_SCAN_DONE: {
                wifi_event_sta_scan_done_t* scan_done = (wifi_event_sta_scan_done_t*) event_data;
                ESP_LOGI(TAG, "Scan done: %d networks found", scan_done->number);
                break;
            }

            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Station started");
                break;

            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "Station stopped");
                break;

            case WIFI_EVENT_STA_CONNECTED: {
                wifi_event_sta_connected_t* connected = (wifi_event_sta_connected_t*) event_data;
                mac_address_to_str(connected->bssid, mac_str, sizeof(mac_str));
                ESP_LOGI(TAG, "Station connected to AP, SSID: %.*s, BSSID: %s",
                         connected->ssid_len, (char*)connected->ssid, mac_str);
                break;
            }

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
                ESP_LOGI(TAG, "Station disconnected, reason: %d", disconnected->reason);
                break;
            }

            case WIFI_EVENT_STA_AUTHMODE_CHANGE: {
                wifi_event_sta_authmode_change_t* auth_change = (wifi_event_sta_authmode_change_t*) event_data;
                ESP_LOGI(TAG, "Authmode changed from %d to %d", auth_change->old_mode, auth_change->new_mode);
                break;
            }

            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "AP started");
                break;

            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "AP stopped");
                break;

            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* sta_connected = (wifi_event_ap_staconnected_t*) event_data;
                mac_address_to_str(sta_connected->mac, mac_str, sizeof(mac_str));
                ESP_LOGI(TAG, "AP station connected, AID=%d, MAC=%s", sta_connected->aid, mac_str);
                break;
            }

            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* sta_disconnected = (wifi_event_ap_stadisconnected_t*) event_data;
                mac_address_to_str(sta_disconnected->mac, mac_str, sizeof(mac_str));
                ESP_LOGI(TAG, "AP station disconnected, AID=%d, MAC=%s", sta_disconnected->aid, mac_str);
                break;
            }

            case WIFI_EVENT_AP_PROBEREQRECVED: {
                wifi_event_ap_probe_req_rx_t* probe_req = (wifi_event_ap_probe_req_rx_t*) event_data;
                mac_address_to_str(probe_req->mac, mac_str, sizeof(mac_str));
                ESP_LOGI(TAG, "Probe request received, RSSI=%d, MAC=%s", probe_req->rssi, mac_str);
                break;
            }

            default:
                ESP_LOGW(TAG, "Unhandled WIFI_EVENT id %d", (int)event_id);
                break;
        }
    } 
    else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* ip_info = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "Station got IP: " IPSTR, IP2STR(&ip_info->ip_info.ip));
                break;
            }

            case IP_EVENT_STA_LOST_IP:
                ESP_LOGI(TAG, "Station lost IP");
                break;

            case IP_EVENT_AP_STAIPASSIGNED:
                ESP_LOGI(TAG, "AP assigned IP to station");
                break;

            default:
                ESP_LOGW(TAG, "Unhandled IP_EVENT id %d", (int)event_id);
                break;
        }
    }
    else if (event_base == ETH_EVENT) {
        switch (event_id) {
            case ETHERNET_EVENT_START:
                ESP_LOGI(TAG, "Ethernet started");
                break;

            case ETHERNET_EVENT_STOP:
                ESP_LOGI(TAG, "Ethernet stopped");
                break;

            case ETHERNET_EVENT_CONNECTED:
                ESP_LOGI(TAG, "Ethernet connected");
                break;

            case ETHERNET_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "Ethernet disconnected");
                break;

            default:
                ESP_LOGW(TAG, "Unhandled ETHERNET_EVENT id %d", (int)event_id);
                break;
        }
    }
    else {
        ESP_LOGW(TAG, "Unhandled event base %s id %d", event_base, (int)event_id);
    }
}
