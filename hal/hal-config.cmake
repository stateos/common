include_guard(GLOBAL)

if (NOT DEFINED TARGET_DEVICE)
    message(FATAL_ERROR "Please define TARGET_DEVICE before including hal package")
endif()

project(hal)

add_library(hal INTERFACE)

target_compile_definitions(hal INTERFACE USE_HAL_DRIVER)

include(${CMAKE_CURRENT_LIST_DIR}/${TARGET_DEVICE}/config.cmake)
