include_guard(GLOBAL)

if (NOT DEFINED TARGET_COMPILER)
    message(FATAL_ERROR "Please define TARGET_COMPILER before including the cmsis::os package")
endif()

project(cmsis-os)

add_library(cmsis-os INTERFACE)
add_library(cmsis::os ALIAS cmsis-os)
add_library(cmsis-rtx STATIC IMPORTED)
set_target_properties(cmsis-rtx
	PROPERTIES
	IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${TARGET_COMPILER}/${CMAKE_STATIC_LIBRARY_PREFIX}RTX_CM4F${CMAKE_STATIC_LIBRARY_SUFFIX}
)
target_link_libraries(cmsis-os INTERFACE cmsis::core cmsis-rtx)

target_sources(cmsis-os
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/os_systick.c
)

target_include_directories(cmsis-os
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/inc
)
