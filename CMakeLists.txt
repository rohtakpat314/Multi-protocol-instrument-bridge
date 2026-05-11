cmake_minimum_required(VERSION 3.13)

option(BRIDGE_HOST_TESTS "Build host-side parser tests instead of firmware" OFF)

if(NOT BRIDGE_HOST_TESTS)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake")
        include("${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake")
    elseif(DEFINED ENV{PICO_SDK_PATH} AND EXISTS "$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")
        include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")
    else()
        message(FATAL_ERROR "Pico SDK not found. Set PICO_SDK_PATH or copy pico_sdk_import.cmake into this directory.")
    endif()
endif()

project(probebridge C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(BRIDGE_HOST_TESTS)
    add_executable(parser_tests
        tests/parser_tests.c
        src/command.c
    )
    target_include_directories(parser_tests PRIVATE include)
    target_compile_definitions(parser_tests PRIVATE BRIDGE_HOST_TESTS=1)
else()
    pico_sdk_init()

    add_executable(probebridge
        src/main.c
        src/command.c
        src/bus_pico.c
        src/line_editor.c
    )

    target_include_directories(probebridge PRIVATE include)
    target_link_libraries(probebridge
        pico_stdlib
        hardware_gpio
        hardware_i2c
        hardware_spi
        hardware_uart
    )

    pico_enable_stdio_usb(probebridge 1)
    pico_enable_stdio_uart(probebridge 0)
    pico_add_extra_outputs(probebridge)
endif()
