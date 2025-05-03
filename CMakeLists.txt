cmake_minimum_required(VERSION 3.20.0)

# Optional: set device tree overlay file
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/app.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(agriprobe_biobox)

target_include_directories(app PRIVATE include)
target_sources(app PRIVATE src/main.c)

# Build other sources from src/
add_subdirectory(src)

