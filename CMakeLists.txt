cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/pico_sdk_import.cmake)

project(pico2_bootchain)
pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(firmware)