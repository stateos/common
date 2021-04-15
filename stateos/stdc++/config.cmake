include_guard(GLOBAL)

if (NOT DEFINED STATEOS_PATH)
	message(FATAL_ERROR "Please define STATEOS_PATH before including the stateos::cmsis package")
endif()

project(stateos-stdc++)

add_library(stateos-stdc++ INTERFACE)
add_library(stateos::stdc++ ALIAS stateos-stdc++)
target_link_libraries(stateos-stdc++ INTERFACE stateos::kernel)

target_compile_definitions(stateos-stdc++
	INTERFACE
	_GLIBCXX_HAS_GTHREADS
)

target_sources(stateos-stdc++
	INTERFACE
	${STATEOS_PATH}/stdc++/src/emutls.cc
	${STATEOS_PATH}/stdc++/src/chrono.cc
	${STATEOS_PATH}/stdc++/src/future.cc
	${STATEOS_PATH}/stdc++/src/mutex.cc
	${STATEOS_PATH}/stdc++/src/condition_variable.cc
	${STATEOS_PATH}/stdc++/src/thread.cc
)

target_include_directories(stateos-stdc++
	INTERFACE
	${STATEOS_PATH}/stdc++
)
