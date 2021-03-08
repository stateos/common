include_guard(GLOBAL)

if (NOT DEFINED INTROS_PATH)
	set(INTROS_PATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

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
	${INTROS_PATH}/kernel/oskernel.c
	${INTROS_PATH}/kernel/src/osclock.c
	${INTROS_PATH}/kernel/src/osbarrier.c
	${INTROS_PATH}/kernel/src/osconditionvariable.c
	${INTROS_PATH}/kernel/src/osevent.c
	${INTROS_PATH}/kernel/src/oseventqueue.c
	${INTROS_PATH}/kernel/src/osflag.c
	${INTROS_PATH}/kernel/src/osjobqueue.c
	${INTROS_PATH}/kernel/src/oslist.c
	${INTROS_PATH}/kernel/src/osmailboxqueue.c
	${INTROS_PATH}/kernel/src/osmemorypool.c
	${INTROS_PATH}/kernel/src/osmessagebuffer.c
	${INTROS_PATH}/kernel/src/osmutex.c
	${INTROS_PATH}/kernel/src/osrwlock.c
	${INTROS_PATH}/kernel/src/ossemaphore.c
	${INTROS_PATH}/kernel/src/ossignal.c
	${INTROS_PATH}/kernel/src/osstreambuffer.c
	${INTROS_PATH}/kernel/src/ostask.c
	${INTROS_PATH}/kernel/src/ostimer.c
)

if (EXISTS ${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.s)
target_sources(intros
	INTERFACE
	${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.s
)
endif()

if (EXISTS ${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.c)
target_sources(intros
	INTERFACE
	${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oscore.c
)
endif()

if (EXISTS ${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oslib.c)
target_sources(intros
	INTERFACE
	${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}/oslib.c
)
endif()

if (EXISTS ${INTROS_PATH}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}/osport.c)
target_sources(intros
	INTERFACE
	${INTROS_PATH}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}/osport.c
)
endif()

target_include_directories(intros
	INTERFACE
	${INTROS_PATH}/kernel
	${INTROS_PATH}/port/${INTROS_ARCH}/compiler/${INTROS_COMPILER}
	${INTROS_PATH}/port/${INTROS_ARCH}/device/${INTROS_DEVICE}
)

if (EXISTS ${INTROS_PATH}/port/${INTROS_ARCH}/common)
target_include_directories(intros
	INTERFACE
	${INTROS_PATH}/port/${INTROS_ARCH}/common
)
endif()
