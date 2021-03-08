include_guard(GLOBAL)

add_library(device::attiny817xmini ALIAS device)

target_sources(device
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/init.c
)

target_include_directories(device
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)
