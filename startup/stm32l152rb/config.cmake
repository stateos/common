include_guard(GLOBAL)

if (NOT DEFINED TARGET_COMPILER)
    message(FATAL_ERROR "Please define TARGET_COMPILER before including the startup package")
endif()

target_sources(startup
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/system.c
	${CMAKE_CURRENT_LIST_DIR}/startup.c
)

include("${CMAKE_CURRENT_LIST_DIR}/${TARGET_COMPILER}/config.cmake")
