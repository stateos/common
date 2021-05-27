include_guard(GLOBAL)

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
	${CMAKE_CURRENT_LIST_DIR}/src/emutls.cc
	${CMAKE_CURRENT_LIST_DIR}/src/chrono.cc
	${CMAKE_CURRENT_LIST_DIR}/src/future.cc
	${CMAKE_CURRENT_LIST_DIR}/src/mutex.cc
	${CMAKE_CURRENT_LIST_DIR}/src/condition_variable.cc
	${CMAKE_CURRENT_LIST_DIR}/src/thread.cc
)

target_include_directories(stateos-stdc++
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)
