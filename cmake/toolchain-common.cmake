include_guard(GLOBAL)

set(STARTMSG "■")
message(STATUS "${STARTMSG} Cmake version: ${CMAKE_VERSION}")
message(STATUS "${STARTMSG} Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "${STARTMSG} Platform: ${TARGET_PLATFORM}")
message(STATUS "${STARTMSG} Host: ${CMAKE_HOST_SYSTEM_NAME}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(COMMON ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

list(APPEND CMAKE_PREFIX_PATH ${COMMON})

function(add_common_libraries)
	foreach(library ${ARGN})
		if (EXISTS ${COMMON}/${library}/CMakeLists.txt)
			add_subdirectory(${COMMON}/${library} "${CMAKE_CURRENT_BINARY_DIR}/common/${library}")
		endif()
	endforeach()
endfunction()

function(add_flags variable)
	if (DEFINED ${variable})
		foreach(flag ${ARGN})
			string(REGEX REPLACE "(^| +)${flag}[^ ]*" "" value "${${variable}}")
			set_property(CACHE ${variable} PROPERTY VALUE "${value} ${flag}")
		endforeach()
	else()
		set(${variable} "${flag}" CACHE INTERNAL "${variable}")
	endif()
endfunction()

function(remove_flags variable)
	if (DEFINED ${variable})
		foreach(flag ${ARGN})
			string(REGEX REPLACE "(^| +)${flag}[^ ]*" "" value "${${variable}}")
			set_property(CACHE ${variable} PROPERTY VALUE "${value}")
		endforeach()
	endif()
endfunction()

function(update_flags variable)
	if (DEFINED ${variable})
		foreach(flag ${ARGN})
			string(REGEX MATCH "[^=]+" var ${flag})
			string(REGEX REPLACE "(^| +)${var}[^ ]*" "" value "${${variable}}")
			set_property(CACHE ${variable} PROPERTY VALUE "${value} ${flag}")
		endforeach()
	else()
		set(${variable} "${flag}" CACHE INTERNAL "${variable}")
	endif()
endfunction()

function(add_target_flags target property)
	foreach(flag ${ARGN})
		get_target_property(value ${target} ${property})
		list(FILTER value EXCLUDE REGEX ${flag})
		list(APPEND value ${flag})
		set_target_properties(${target} PROPERTIES ${property} "${value}")
	endforeach()
endfunction()

function(remove_target_flags target property)
	foreach(flag ${ARGN})
		get_target_property(value ${target} ${property})
		list(FILTER value EXCLUDE REGEX ${flag})
		set_target_properties(${target} PROPERTIES ${property} "${value}")
	endforeach()
endfunction()

function(update_target_flags target property)
	foreach(flag ${ARGN})
		get_target_property(value ${target} ${property})
		string(REGEX MATCH "[^=]+" var ${flag})
		list(FILTER value EXCLUDE REGEX ${var})
		list(APPEND value ${flag})
		set_target_properties(${target} PROPERTIES ${property} "${value}")
	endforeach()
endfunction()
