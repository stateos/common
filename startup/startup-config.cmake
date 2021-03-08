include_guard(GLOBAL)

if (NOT DEFINED TARGET_CHIP)
    message(FATAL_ERROR "Please define TARGET_CHIP before including the startup package")
endif()

project(startup)

add_library(startup INTERFACE)

include("${CMAKE_CURRENT_LIST_DIR}/${TARGET_CHIP}/config.cmake")
