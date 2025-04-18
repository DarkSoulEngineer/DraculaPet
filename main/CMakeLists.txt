# source files
set(SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    )

set(INCLUDE_EXTERNAL_DIRS . include)
set(REQUIRED_MODULES esp_wifi nvs_flash wifi_controller command_line)

# component
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_EXTERNAL_DIRS}
                    REQUIRES ${REQUIRED_MODULES})