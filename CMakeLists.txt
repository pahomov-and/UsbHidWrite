cmake_minimum_required(VERSION 3.0)
project(UsbHidWrite)

set(CMAKE_CXX_STANDARD 14)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

include_directories(.)

add_executable(${PROJECT_NAME}
        main.cpp
        USB.h
        USB.cpp
        )
target_link_libraries(${PROJECT_NAME} usb usb-1.0)
