# CMakeLists.txt
cmake_minimum_required(VERSION 3.5)

# Include the project CMakeLists.txt file
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS components)

# Configure the cpp standard
set(CMAKE_CXX_STANDARD 17)

# Set the project name
project(Dracula)