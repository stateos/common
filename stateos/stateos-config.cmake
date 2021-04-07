include_guard(GLOBAL)

if (NOT DEFINED STATEOS_PATH)
	set(STATEOS_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

if (NOT DEFINED STATEOS_ARCH)
    message(FATAL_ERROR "Please define STATEOS_ARCH before including the stateos::kernel package")
endif()

if (NOT DEFINED STATEOS_DEVICE)
    message(FATAL_ERROR "Please define STATEOS_DEVICE before including the stateos::kernel package")
endif()

if (NOT DEFINED STATEOS_COMPILER)
    message(FATAL_ERROR "Please define STATEOS_COMPILER before including the stateos::kernel package")
endif()

project(stateos)

find_package(cmsis REQUIRED)
add_library(stateos INTERFACE)
add_library(stateos::kernel ALIAS stateos)
target_link_libraries(stateos INTERFACE cmsis)

target_sources(stateos
	INTERFACE
	${STATEOS_PATH}/kernel/osalloc.c
	${STATEOS_PATH}/kernel/oskernel.c
	${STATEOS_PATH}/kernel/src/osclock.c
	${STATEOS_PATH}/kernel/src/osbarrier.c
	${STATEOS_PATH}/kernel/src/osconditionvariable.c
	${STATEOS_PATH}/kernel/src/osevent.c
	${STATEOS_PATH}/kernel/src/oseventqueue.c
	${STATEOS_PATH}/kernel/src/osfastmutex.c
	${STATEOS_PATH}/kernel/src/osflag.c
	${STATEOS_PATH}/kernel/src/osjobqueue.c
	${STATEOS_PATH}/kernel/src/oslist.c
	${STATEOS_PATH}/kernel/src/osmailboxqueue.c
	${STATEOS_PATH}/kernel/src/osmemorypool.c
	${STATEOS_PATH}/kernel/src/osmessagebuffer.c
	${STATEOS_PATH}/kernel/src/osmutex.c
	${STATEOS_PATH}/kernel/src/osrwlock.c
	${STATEOS_PATH}/kernel/src/ossemaphore.c
	${STATEOS_PATH}/kernel/src/ossignal.c
	${STATEOS_PATH}/kernel/src/osstreambuffer.c
	${STATEOS_PATH}/kernel/src/ostask.c
	${STATEOS_PATH}/kernel/src/ostimer.c
)

if (EXISTS ${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.s)
target_sources(stateos
	INTERFACE
	${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.s
)
endif()

if (EXISTS ${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.c)
target_sources(stateos
	INTERFACE
	${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.c
)
endif()

if (EXISTS ${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oslibc.c)
target_sources(stateos
	INTERFACE
	${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oslibc.c
)
endif()

if (EXISTS ${STATEOS_PATH}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}/osport.c)
target_sources(stateos
	INTERFACE
	${STATEOS_PATH}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}/osport.c
)
endif()

target_include_directories(stateos
	INTERFACE
	${STATEOS_PATH}/kernel
	${STATEOS_PATH}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}
	${STATEOS_PATH}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}
)

if (EXISTS ${STATEOS_PATH}/port/${STATEOS_ARCH}/common)
target_include_directories(stateos
	INTERFACE
	${STATEOS_PATH}/port/${STATEOS_ARCH}/common
)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmsis/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/nasa/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/std/config.cmake)
