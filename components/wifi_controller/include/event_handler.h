#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include "esp_event.h"

// Helper: Convert MAC to string
void mac_address_to_str(const uint8_t* mac, char* str, size_t str_len);

// New-style event handler function signature
void wifi_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif // EVENT_HANDLER_H
