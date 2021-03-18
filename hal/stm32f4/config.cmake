include_guard(GLOBAL)

if (NOT DEFINED TARGET_DEVICE)
    message(FATAL_ERROR "Please define TARGET_DEVICE before including hal package")
endif()

if (NOT DEFINED TARGET_COMPILER)
    message(FATAL_ERROR "Please define TARGET_COMPILER before including hal package")
endif()

find_package(cmsis REQUIRED)
target_link_libraries(hal INTERFACE cmsis)
add_library(hal::${TARGET_DEVICE} ALIAS hal)
add_library(hal-${TARGET_DEVICE} STATIC IMPORTED)
set_target_properties(hal-${TARGET_DEVICE}
	PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${TARGET_COMPILER}/${CMAKE_STATIC_LIBRARY_PREFIX}HAL${CMAKE_STATIC_LIBRARY_SUFFIX}
)
target_link_libraries(hal INTERFACE hal-${TARGET_DEVICE})

target_include_directories(hal
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/inc
)
