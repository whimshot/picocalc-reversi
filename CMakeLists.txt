# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picocalc-reversi C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ChaN's FatFs (http://elm-chan.org/fsw/ff/) for the SD card. The Pico SDK
# carries a copy for TinyUSB. ff.h includes "ffconf.h" from its own
# directory, so the sources are copied into the build tree beside our
# drivers/fatfs/ffconf.h rather than compiled in place (which would find
# TinyUSB's configuration).
set(FATFS_SRC ${PICO_SDK_PATH}/lib/tinyusb/lib/fatfs/source)
set(FATFS_DIR ${CMAKE_BINARY_DIR}/fatfs)
foreach(f ff.c ff.h ffunicode.c diskio.h)
    if (NOT EXISTS ${FATFS_SRC}/${f})
        message(FATAL_ERROR "FatFs ${f} not found in ${FATFS_SRC}")
    endif()
    configure_file(${FATFS_SRC}/${f} ${FATFS_DIR}/${f} COPYONLY)
endforeach()
configure_file(drivers/fatfs/ffconf.h ${FATFS_DIR}/ffconf.h COPYONLY)

add_library(fatfs STATIC
        ${FATFS_DIR}/ff.c
        ${FATFS_DIR}/ffunicode.c
        )
target_include_directories(fatfs PUBLIC ${FATFS_DIR})

# Add executable. Default name is the project name, version 0.1

add_executable(picocalc-reversi
        picocalc-reversi.c
        picocalc-reversi.h
        graphic-game-board.c
        graphic-game-board.h
        drivers/audio.c
        drivers/audio.h
        drivers/clib.c
        drivers/diskio.c
        drivers/display.c
        drivers/display.h
        drivers/filesystem.c
        drivers/filesystem.h
        drivers/font-5x10.c
        drivers/font-8x10.c
        drivers/font.h
        drivers/keyboard.c
        drivers/keyboard.h
        drivers/lcd.c
        drivers/lcd.h
        drivers/onboard_led.c
        drivers/onboard_led.h
        drivers/picocalc.c
        drivers/picocalc.h
        drivers/sdcard.c
        drivers/sdcard.h
        drivers/southbridge.c
        drivers/southbridge.h
        )

pico_set_program_name(picocalc-reversi "Reversi for the PicoCalc")
pico_set_program_version(picocalc-reversi "v2.0.3")

# Generate PIO header
pico_generate_pio_header(picocalc-reversi ${CMAKE_CURRENT_LIST_DIR}/drivers/audio.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picocalc-reversi 0)
pico_enable_stdio_usb(picocalc-reversi 1)

# Add the standard library to the build
target_link_libraries(picocalc-reversi
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(picocalc-reversi PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/third_party/picocalc-shell
)

# The PicoCalc shell's clock, for file dates, is in the RP2350's always-on
# timer (POWMAN). The RP2040 has none, and its files are left undated.
if (TARGET hardware_powman)
    target_link_libraries(picocalc-reversi hardware_powman)
endif()
MESSAGE( STATUS "cmake current list dir: ${CMAKE_CURRENT_LIST_DIR}" )

# Add any user requested libraries
target_link_libraries(picocalc-reversi
        fatfs
        pico_printf
        pico_float
        pico_status_led
        pico_rand
        hardware_gpio
        hardware_i2c
        hardware_spi
        hardware_pio
        hardware_clocks
        )

pico_add_extra_outputs(picocalc-reversi)
