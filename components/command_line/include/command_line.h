#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <ctype.h>


#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024

#define STATE_INITIAL 0
#define STATE_AP_SELECTION 1
#define STATE_COMMAND_MODE 2

typedef struct {
    bool keep_running;
    int state;
} uart_task_args_t;

void handle_ap_selection(const char* input, bool* keep_running);
void process_input(const char* input, bool* keep_running);
void read_user_input(int input_state);
void serial_comm_config(void);
void read_uart(void *arg);
void ap_scan();
#endif
