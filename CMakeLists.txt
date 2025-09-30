cmake_minimum_required(VERSION 3.13)

# Set board type to Pico W for WiFi support
set(PICO_BOARD pico_w)

# Include the Pico SDK initialization script
include(${CMAKE_SOURCE_DIR}/lib/pico-sdk/pico_sdk_init.cmake)

project(my_project)

# Initialize the Pico SDK
pico_sdk_init()

# Add your executable and source files
add_executable(my_project
    src/main.c
    src/settings.c
    src/midi.c
    src/mcp3008_reader.c
    src/status_dispatcher.c
    src/access_point.c
    src/dhcpserver.c
    src/dnsserver.c
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(my_project 1)
pico_enable_stdio_uart(my_project 0)

# Add include directory for lwipopts.h
target_include_directories(my_project PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link the Pico SDK to your project including WiFi and networking libraries
target_link_libraries(my_project 
    pico_stdlib 
    hardware_spi 
    pico_multicore
    pico_cyw43_arch_lwip_poll
    pico_lwip_http
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(my_project)