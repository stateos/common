include_guard(GLOBAL)

if (NOT DEFINED CMSISOS_COMPILER)
	message(FATAL_ERROR "Please define CMSISOS_COMPILER before including the cmsis::os package")
endif()

if (NOT DEFINED CMSISOS_PORTFILE)
	message(FATAL_ERROR "Please define CMSISOS_PORTFILE before including the cmsis::os package")
endif()

project(cmsis-os)

add_library(cmsis-os INTERFACE)
add_library(cmsis::os ALIAS cmsis-os)

target_sources(cmsis-os
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/os_systick.c
	${CMAKE_CURRENT_LIST_DIR}/src/os_systick.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/config/RTX_Config.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_delay.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_evflags.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_evr.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_kernel.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_lib.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_memory.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_mempool.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_msgqueue.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_mutex.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_semaphore.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_system.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_thread.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/rtx_timer.c
	${CMAKE_CURRENT_LIST_DIR}/rtx/source/${CMSISOS_COMPILER}/${CMSISOS_PORTFILE}
)

target_include_directories(cmsis-os
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/inc
	${CMAKE_CURRENT_LIST_DIR}/rtx/config
	${CMAKE_CURRENT_LIST_DIR}/rtx/include
)
