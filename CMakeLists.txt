cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK initialization script
include(${CMAKE_SOURCE_DIR}/lib/pico-sdk/pico_sdk_init.cmake)

project(my_project)

# Initialize the Pico SDK
pico_sdk_init()

# Add your executable and source files
add_executable(my_project
    main.c
)

# Enable USB stdio and disable UART stdio
pico_enable_stdio_usb(my_project 1)
pico_enable_stdio_uart(my_project 0)

# Link the Pico SDK to your project
target_link_libraries(my_project pico_stdlib hardware_spi)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(my_project)