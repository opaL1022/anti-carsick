cmake_minimum_required(VERSION 3.16)

project(MotionOverlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(MotionOverlay
    main.cpp
    overlaywidget.h
    overlaywidget.cpp
    udpreceiver.h
    udpreceiver.cpp
)

target_link_libraries(MotionOverlay PRIVATE
    Qt6::Widgets
    Qt6::Network
)
