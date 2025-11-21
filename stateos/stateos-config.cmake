include_guard(GLOBAL)

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
	${CMAKE_CURRENT_LIST_DIR}/kernel/oskernel.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/osalloc.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/ossys.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osclock.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osbarrier.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osconditionvariable.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osevent.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/oseventqueue.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osflag.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osjobqueue.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/oslist.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osmailboxqueue.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osmemorypool.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osmessagequeue.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osmutex.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osrwlock.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osrawbuffer.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/ossemaphore.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/ossignal.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/ostask.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/ostimer.c
	${CMAKE_CURRENT_LIST_DIR}/kernel/src/osstatemachine.c
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.s)
target_sources(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.s
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.c)
target_sources(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oscore.c
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oslibc.c)
target_sources(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}/oslibc.c
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}/osport.c)
target_sources(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}/osport.c
)
endif()

target_include_directories(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/kernel
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/compiler/${STATEOS_COMPILER}
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/device/${STATEOS_DEVICE}
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/common)
target_include_directories(stateos
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${STATEOS_ARCH}/common
)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/stdc++/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmsis/config.cmake)
