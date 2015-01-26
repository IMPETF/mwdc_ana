cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(mwdc_ana)

add_subdirectory(kernel ${PROJECT_BINARY_DIR})
add_subdirectory(scripts ${PROJECT_BINARY_DIR})

add_custom_target(basic DEPENDS kernel scripts)
