include_guard(GLOBAL)

if (NOT DEFINED TARGET_COMPILER)
    message(FATAL_ERROR "Please define TARGET_COMPILER before including the device::semihost package")
endif()

project(device-semihost)

add_library(device-semihost INTERFACE)
add_library(device::semihost ALIAS device-semihost)
target_link_libraries(device-semihost INTERFACE device device::nosys)

target_include_directories(device-semihost
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)

target_sources(device-semihost
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/semihost.c
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TARGET_COMPILER})
target_sources(device-semihost
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/${TARGET_COMPILER}/retarget.c
)
endif()
