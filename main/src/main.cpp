#include "main.h"

// C Modules
extern "C" {
    #include "ap_scanner.h"   // Handle AP scanning
    #include "command_line.h" // Handle command line input
}


void app_main() {             // Start of main function
    read_user_input(1);       // Call the function to read user input
}
