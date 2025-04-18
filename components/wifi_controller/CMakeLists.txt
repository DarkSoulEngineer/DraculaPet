# source files
set(SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/wifi_controller.c
    ${CMAKE_CURRENT_LIST_DIR}/src/ap_scanner.c
    ${CMAKE_CURRENT_LIST_DIR}/src/sniffer.c
)
set(INCLUDE_EXTERNAL_DIRS . include)
set(REQUIRED_MODULES nvs_flash esp_wifi)

# component
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_EXTERNAL_DIRS}
                    REQUIRES ${REQUIRED_MODULES})