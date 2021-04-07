include_guard(GLOBAL)

if (NOT DEFINED STATEOS_PATH)
	message(FATAL_ERROR "Please define STATEOS_PATH before including the stateos::cmsis package")
endif()

project(stateos-std)

add_library(stateos-std INTERFACE)
add_library(stateos::std ALIAS stateos-std)
target_link_libraries(stateos-std INTERFACE stateos::kernel)

target_compile_definitions(stateos-std
	INTERFACE
	_GLIBCXX_HAS_GTHREADS
)

target_sources(stateos-std
	INTERFACE
	${STATEOS_PATH}/std/src/chrono.cc
	${STATEOS_PATH}/std/src/condition_variable.cc
	${STATEOS_PATH}/std/src/future.cc
	${STATEOS_PATH}/std/src/mutex.cc
	${STATEOS_PATH}/std/src/thread.cc
)

target_include_directories(stateos-std
	INTERFACE
	${STATEOS_PATH}/std
)
