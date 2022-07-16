include_guard(GLOBAL)

if (NOT DEFINED INTROS_ARCH)
    message(FATAL_ERROR "Please define INTROS_ARCH before including the intros::kernel package")
endif()

if (NOT DEFINED INTROS_DEVICE)
    message(FATAL_ERROR "Please define INTROS_DEVICE before including the intros::kernel package")
endif()

if (NOT DEFINED INTROS_COMPILER)
    message(FATAL_ERROR "Please define INTROS_COMPILER before including the intros::kernel package")
endif()

project(intros)

find_package(cmsis REQUIRED)
add_library(intros INTERFACE)
add_library(intros::kernel ALIAS intros)
target_link_libraries(intros INTERFACE cmsis)

target_sources(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/kernel/oskernel.c
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

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.s)
target_sources(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.s
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.c)
target_sources(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.c
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oslib.c)
target_sources(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oslib.c
)
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}/osport.c)
target_sources(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}/osport.c
)
endif()

target_include_directories(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/kernel
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}
)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/common)
target_include_directories(intros
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/port/${INTROS_ARCH}/common
)
endif()
