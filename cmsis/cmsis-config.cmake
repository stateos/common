include_guard(GLOBAL)

if (NOT DEFINED TARGET_DEVICE)
    message(FATAL_ERROR "Please define TARGET_DEVICE before including cmsis::core package")
endif()

add_library(cmsis INTERFACE)
add_library(cmsis::core ALIAS cmsis)

target_include_directories(cmsis
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/${TARGET_DEVICE}
)

include(${CMAKE_CURRENT_LIST_DIR}/os/config.cmake)
