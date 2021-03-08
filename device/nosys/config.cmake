include_guard(GLOBAL)

if (NOT DEFINED TARGET_ARCH)
    message(FATAL_ERROR "Please define TARGET_ARCH before including the device package")
endif()

if (NOT DEFINED TARGET_COMPILER)
    message(FATAL_ERROR "Please define TARGET_COMPILER before including the device package")
endif()

project(device-nosys)

add_library(device-nosys INTERFACE)
add_library(device::nosys ALIAS device-nosys)
target_link_libraries(device-nosys INTERFACE device)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TARGET_ARCH}/${TARGET_COMPILER}/retarget.c)
target_sources(device-nosys
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/${TARGET_ARCH}/${TARGET_COMPILER}/retarget.c
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/${TARGET_ARCH}/${TARGET_COMPILER}/${CMAKE_STATIC_LIBRARY_PREFIX}retarget${CMAKE_STATIC_LIBRARY_SUFFIX})
add_library(device-retarget STATIC IMPORTED)
set_target_properties(device-retarget
	PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${TARGET_ARCH}/${TARGET_COMPILER}/${CMAKE_STATIC_LIBRARY_PREFIX}retarget${CMAKE_STATIC_LIBRARY_SUFFIX}
)
target_link_libraries(device-nosys INTERFACE device-retarget)
endif()
