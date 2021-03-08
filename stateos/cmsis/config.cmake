include_guard(GLOBAL)

if (NOT DEFINED STATEOS_PATH)
	message(FATAL_ERROR "Please define STATEOS_PATH before including the stateos::cmsis package")
endif()

project(stateos-cmsis)

add_library(stateos-cmsis INTERFACE)
add_library(stateos::cmsis ALIAS stateos-cmsis)
target_link_libraries(stateos-cmsis INTERFACE stateos::kernel)

target_sources(stateos-cmsis
	INTERFACE
	${STATEOS_PATH}/cmsis/src/cmsis_os1.c
	${STATEOS_PATH}/cmsis/src/cmsis_os2.c
)

target_include_directories(stateos-cmsis
	INTERFACE
	${STATEOS_PATH}/cmsis/inc
)
