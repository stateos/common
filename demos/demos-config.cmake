include_guard(GLOBAL)

if (NOT DEFINED DEMOS_ARCH)
    message(FATAL_ERROR "Please define DEMOS_ARCH before including the demos::kernel package")
endif()

if (NOT DEFINED DEMOS_DEVICE)
    message(FATAL_ERROR "Please define DEMOS_DEVICE before including the demos::kernel package")
endif()

project(demos)

find_package(cmsis REQUIRED)
add_library(demos INTERFACE)
add_library(demos::kernel ALIAS demos)
target_link_libraries(demos INTERFACE cmsis)

target_sources(demos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/kernel/os.c
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${DEMOS_ARCH}/${DEMOS_DEVICE}/osport.c)
target_sources(demos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${DEMOS_ARCH}/${DEMOS_DEVICE}/osport.c
)
endif()

target_include_directories(demos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/kernel
	${CMAKE_CURRENT_LIST_DIR}/port/${DEMOS_ARCH}/${DEMOS_DEVICE}
)
