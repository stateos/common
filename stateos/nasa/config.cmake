include_guard(GLOBAL)

if (NOT DEFINED STATEOS_PATH)
	message(FATAL_ERROR "Please define STATEOS_PATH before including the stateos::nasa package")
endif()

project(stateos-nasa)

add_library(stateos-nasa INTERFACE)
add_library(stateos::nasa ALIAS stateos-nasa)
target_link_libraries(stateos-nasa INTERFACE stateos::kernel)

target_sources(stateos-nasa
	INTERFACE
	${STATEOS_PATH}/nasa/bsp/shared/src/bsp_default_app_run.c
	${STATEOS_PATH}/nasa/bsp/shared/src/bsp_default_app_startup.c
	${STATEOS_PATH}/nasa/bsp/shared/src/bsp_default_symtab.c
	${STATEOS_PATH}/nasa/bsp/shared/src/osapi-bsp.c
	${STATEOS_PATH}/nasa/bsp/stateos/src/bsp_console.c
	${STATEOS_PATH}/nasa/bsp/stateos/src/bsp_start.c
	${STATEOS_PATH}/nasa/os/portable/os-impl-console-bsp.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-binsem.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-clock.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-common.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-countsem.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-debug.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-dir.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-errors.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-file.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-filesys.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-heap.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-idmap.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-module.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-mutex.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-network.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-printf.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-queue.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-select.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-shell.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-sockets.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-task.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-time.c
	${STATEOS_PATH}/nasa/os/shared/src/osapi-timebase.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-binsem.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-clock.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-common.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-console.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-countsem.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-errors.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-idmap.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-mutex.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-dirs.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-files.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-filesys.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-heap.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-io.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-loader.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-network.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-select.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-shell.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-sockets.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-no-symtab.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-queues.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-tasks.c
	${STATEOS_PATH}/nasa/os/stateos/src/os-impl-timebase.c
)

target_include_directories(stateos-nasa
	INTERFACE
	${STATEOS_PATH}/nasa/bsp/shared/inc
	${STATEOS_PATH}/nasa/os/inc
	${STATEOS_PATH}/nasa/os/shared/inc
	${STATEOS_PATH}/nasa/os/stateos/inc
)
