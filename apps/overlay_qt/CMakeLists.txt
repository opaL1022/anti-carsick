cmake_minimum_required(VERSION 3.21)

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

set_target_properties(MotionOverlay PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

install(TARGETS MotionOverlay
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(QT_VERSION VERSION_GREATER_EQUAL 6.5)
    qt_generate_deploy_app_script(
        TARGET MotionOverlay
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
