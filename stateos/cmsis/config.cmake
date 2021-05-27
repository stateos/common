include_guard(GLOBAL)

project(stateos-cmsis)

add_library(stateos-cmsis INTERFACE)
add_library(stateos::cmsis ALIAS stateos-cmsis)
target_link_libraries(stateos-cmsis INTERFACE stateos::kernel)

target_sources(stateos-cmsis
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/cmsis_os1.c
	${CMAKE_CURRENT_LIST_DIR}/src/cmsis_os2.c
)

target_include_directories(stateos-cmsis
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/inc
)
