include_guard(GLOBAL)

project(stateos-nasa)

add_library(stateos-nasa INTERFACE)
add_library(stateos::nasa ALIAS stateos-nasa)
target_link_libraries(stateos-nasa INTERFACE stateos::kernel)

target_sources(stateos-nasa
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/bsp/shared/src/bsp_default_app_run.c
	${CMAKE_CURRENT_LIST_DIR}/bsp/shared/src/bsp_default_app_startup.c
	${CMAKE_CURRENT_LIST_DIR}/bsp/shared/src/bsp_default_symtab.c
	${CMAKE_CURRENT_LIST_DIR}/bsp/shared/src/osapi-bsp.c
	${CMAKE_CURRENT_LIST_DIR}/bsp/stateos/src/bsp_console.c
	${CMAKE_CURRENT_LIST_DIR}/bsp/stateos/src/bsp_start.c
	${CMAKE_CURRENT_LIST_DIR}/os/portable/os-impl-console-bsp.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-binsem.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-clock.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-common.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-countsem.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-debug.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-dir.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-errors.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-file.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-filesys.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-heap.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-idmap.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-module.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-mutex.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-network.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-printf.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-queue.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-select.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-shell.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-sockets.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-task.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-time.c
	${CMAKE_CURRENT_LIST_DIR}/os/shared/src/osapi-timebase.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-binsem.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-clock.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-common.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-console.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-countsem.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-errors.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-idmap.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-mutex.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-dir.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-file.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-filesys.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-heap.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-io.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-loader.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-network.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-select.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-shell.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-sockets.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-no-symtab.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-queue.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-task.c
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/src/os-impl-timebase.c
)

target_include_directories(stateos-nasa
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/bsp/shared/inc
	${CMAKE_CURRENT_LIST_DIR}/os/inc
	${CMAKE_CURRENT_LIST_DIR}/os/shared/inc
	${CMAKE_CURRENT_LIST_DIR}/os/stateos/inc
)
