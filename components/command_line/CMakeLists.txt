# source files
set(SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/command_line.c
)
set(INCLUDE_EXTERNAL_DIRS . include)
set(REQUIRED_MODULES driver wifi_controller)

# component
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDE_EXTERNAL_DIRS}
                    REQUIRES ${REQUIRED_MODULES})