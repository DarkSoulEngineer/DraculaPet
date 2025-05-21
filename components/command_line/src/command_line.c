#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "command_line.h"
#include "wifi_controller.h"
#include "ap_scanner.h"

static const char *TAG = "serial_comm";


void serial_comm_config(void) {                                                      // UART configuration
    uart_config_t uart_config = {
        .baud_rate = 115200,                                                         // Baud rate
        .data_bits = UART_DATA_8_BITS,                                               // Data bits
        .parity = UART_PARITY_DISABLE,                                               // Parity
        .stop_bits = UART_STOP_BITS_1,                                               // Stop bits
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,                                       // Flow control
        .rx_flow_ctrl_thresh = 0,                                                    // RX flow control threshold
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));                      // Configure UART parameters
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 10, NULL, 0)); // Install UART driver
}

void read_uart(void *arg) {
    uart_task_args_t *task_args = (uart_task_args_t *)arg;
    bool *keep_running = &(task_args->keep_running);                                 // Use bool instead of enum
    int *state = &(task_args->state);                                                // Use int instead of enum
                
    uint8_t data[1];                                                                 // Store one character
    char buffer[128] = {0};                                                          // Store full input
    int index = 0;                                                                   // Buffer index
                
    // Print State Message                       
    if (*state == STATE_AP_SELECTION) {                                              // Print State Message
        ap_scan();
        printf("Please select an AP: \n");
        // ESP_LOGI(TAG, "Please select an AP: \n");
    } else if (*state == STATE_COMMAND_MODE) {
        printf("Enter a command [ help ] to see available commands, [ quit | exit ]: \n");
        // ESP_LOGI(TAG, "Enter a command [ help ] to see available commands, [ quit | exit ]: \n");
    } else {
        printf("Unknown state: %d\n", *state);
        // ESP_LOGI(TAG, "Unknown state: %d\n", *state);
    }

    while (*keep_running) {  // Keep looping until keep_running is false
        int len = uart_read_bytes(UART_NUM, data, 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {                                                              // Check if data was read
            char ch = data[0];                                                      // Get the character

            if (index >= sizeof(buffer) - 1) {
                // uart_write_bytes(UART_NUM, "\nBuffer full! Press Enter to process input.\n", 46);
                printf("Buffer full! Press Enter to process input.\n");
                buffer[0] = '\0';                                                   // Clear buffer if full
                index = 0;                                                          // Reset index
                continue;                       
            }                       

            if ((ch == 8 || ch == 127) && index > 0) {                              // Handle backspace (8 = ASCII Backspace, 127 = Delete)
                index--;                                                            // Remove last character from buffer
                buffer[index] = '\0';                                               // Null-terminate string
                uart_write_bytes(UART_NUM, "\b \b", 3);                             // Handle backspace on terminal
            }       

            // Handle Enter (send the message)      
            else if (ch == '\n' || ch == '\r') {        
                if (index > 0) {        
                    buffer[index] = '\0';                                           // Null-terminate string
                    printf("CMD> %s \n", buffer);                                   // Print the user input
                    // ESP_LOGI(TAG, "CMD> %s \n", buffer);         

                    if (*state == STATE_AP_SELECTION) {         
                        handle_ap_selection(buffer, keep_running);                  // Process AP selection
                    } else if (*state == STATE_COMMAND_MODE) {          
                        process_input(buffer, keep_running);                        // Process commands
                    } else {            
                        printf("Invalid state: %d\n", *state);                      // Print error message
                        // ESP_LOGI(TAG, "Invalid state: %d", *state);          
                    }
                    index = 0;                                                      // Reset buffer
                    memset(buffer, 0, sizeof(buffer));                              // Clear buffer
                }
            }
            else {          
                buffer[index++] = ch;                                               // Add character to buffer
                uart_write_bytes(UART_NUM, &ch, 1);                                 // Echo character to terminal
            }
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    if (!*keep_running) {                                                           // If keep_running is false, exit the task
        ESP_LOGI(TAG, "UART reading task is exiting");
        vTaskDelete(NULL);
    }
}

void read_user_input(int input_state) {
    serial_comm_config();

    uart_task_args_t *task_args = malloc(sizeof(uart_task_args_t));                // Allocate memory for UART task args
    if (!task_args) {                                                              // Check if memory allocation was successful
        ESP_LOGE(TAG, "Failed to allocate memory for UART task args");
        return;
    }

    task_args->keep_running = true;                                                // Initialize keep_running to true
    task_args->state = input_state;                                                // Initialize state

    xTaskCreate(read_uart, "read_uart_task", 4096, task_args, 10, NULL);           // Create UART reading task
}

void process_input(const char* input, bool* keep_running) {
    if (strcmp(input, "help") == 0) {
        printf("Commands :\n")     ;
        printf("[ help ]\n")       ;
        printf("[ scan beacon ]\n");
        printf("[ quit | exit ]\n");
        
        // @deprecated 
        // ESP_LOGI(TAG, "User requested help");
        // uart_write_bytes(UART_NUM, "Commands :\n", strlen("Commands :\n"));
        // uart_write_bytes(UART_NUM, "[ help ]\n", strlen("[ help ]\n"));
        // uart_write_bytes(UART_NUM, "[ scan beacon ]\n", strlen("[scan beacon]\n"));
        // uart_write_bytes(UART_NUM, "[ quit | exit ]\n", strlen("[ quit | exit ]\n"));

    } else if (strcmp(input, "scan beacon") == 0) {                                //  Handle "scan beacon"
        ESP_LOGI(TAG, "User requested to scan beacon");

    } else if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {         // Handle "quit" or "exit"
        ESP_LOGI(TAG, "User requested to exit");
        uart_write_bytes(UART_NUM, "Exiting...\n", 11);
        *keep_running = false;                                                     // Set keep_running to false to exit the loop
    
    } else {                                                                       // Unknown command
        ESP_LOGI(TAG, "Unknown command: '%s'", input);                                   
        uart_write_bytes(UART_NUM, "Unknown command\n", 16);
    }
}

void handle_ap_selection(const char* input, bool* keep_running) {
    for (int i = 0; input[i] != '\0'; i++) {                                       // Check if the input is a number
        if (!isdigit((unsigned char)input[i])) {                                   // If not, exit the function
            goto invalid_input;                                                    // Exit the function
        }      
    }      
    
    int index = atoi(input) - 1;                                                   // Convert the input to can used as index
    const wifi_ap_record_t *ap = wifictl_get_ap_record(index);                     // Get the AP record at the specified index
    
    if (ap != NULL) {                                                              // If the AP record is valid;                                                             // Create a message to display the selected AP
        printf("You selected: %s (RSSI: %d dBm)\n", ap->ssid, ap->rssi);
        *keep_running = false;
        return;
    }

invalid_input:                                                                     // If the input is not a number or the index is out of range
    printf("Invalid AP: '%s'\n", input);
    const char* response = "Invalid AP selection. Please choose a valid number from the list.\n";
    printf("%s", response);
    *keep_running = true;
}

void ap_scan(){
    wifictl_scan_nearby_aps();                                                     // Scan for nearby APs
    const wifictl_ap_records_t *records = wifictl_get_ap_records();                // Get the AP records
    if (!records) {                                                                // Check if the scan was successful
        printf("Failed to get WiFi scan results\n");
        return;
    }
    print_ap_list(records);                                                        // Print the AP list
}