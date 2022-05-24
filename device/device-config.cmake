include_guard(GLOBAL)

if (NOT DEFINED TARGET_PLATFORM)
    message(FATAL_ERROR "Please define TARGET_PLATFORM before including the device package")
endif()

project(device)

add_library(device INTERFACE)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TARGET_PLATFORM})
include(${CMAKE_CURRENT_LIST_DIR}/${TARGET_PLATFORM}/config.cmake)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/nosys/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/semihost/config.cmake)
