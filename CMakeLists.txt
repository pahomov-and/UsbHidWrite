cmake_minimum_required(VERSION 3.13)
project(UsbHidWrite)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(${PROJECT_NAME}
        main.cpp
        USB.h
        USB.cpp
        )
target_link_libraries(${PROJECT_NAME} usb usb-1.0)
