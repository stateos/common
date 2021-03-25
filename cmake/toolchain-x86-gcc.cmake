cmake_minimum_required(VERSION 3.18)

include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 20)

set(TOOLCHAIN_gcc      "")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_gcc}objcopy")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_gcc}objdump")
set(CMAKE_SIZE         "${TOOLCHAIN_gcc}size")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(COMMON_OPTIONS "")

add_compile_options(${COMMON_OPTIONS})
add_link_options(${COMMON_OPTIONS} -Wl,--gc-sections)

set(STD_WARNING_FLAGS -Wall)
set(ALL_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wlogical-op $<$<COMPILE_LANGUAGE:CXX>:-Wzero-as-null-pointer-constant>)

include("${CMAKE_CURRENT_LIST_DIR}/toolchain-common.cmake")

function(setup_target_includes target)

	foreach(file ${ARGN})
		target_compile_options(${target} PRIVATE -include ${file})
	endforeach()

endfunction()

function(setup_target_definitions target)

	foreach(def ${ARGN})
		target_compile_definitions(${target} PRIVATE ${def})
		target_link_options(${target} PRIVATE -Wl,--defsym=${def})
	endforeach()

endfunction()

function(setup_target_compiler target)

	if (noexceptions IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using noexceptions")
		target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
	endif()

	if (lto IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using lto")
		target_compile_options(${target} PRIVATE -flto)
		target_link_options(${target} PRIVATE -flto)
	endif()

	if (nowarnings IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using nowarnings")
		target_compile_options(${target} PRIVATE ${STD_WARNING_FLAGS})
		target_link_options(${target} PRIVATE ${STD_WARNING_FLAGS})
	else()
		target_compile_options(${target} PRIVATE ${ALL_WARNING_FLAGS})
		target_link_options(${target} PRIVATE ${ALL_WARNING_FLAGS})
	endif()

	if(CMAKE_GENERATOR STREQUAL "Ninja")
		target_compile_options(${target} PRIVATE -fdiagnostics-color)
	endif()

	if (MINGW)
	set(elf_file ${target}.exe)
	else()
	set(elf_file ${target})
	endif()
	set(map_file ${target}.map)
	set(lss_file ${target}.lss)
	set(dmp_file ${target}.dmp)

	target_link_options(${target} PRIVATE -Wl,-Map=${map_file})

	add_custom_command(
		TARGET  ${target}
		POST_BUILD
		COMMAND ${CMAKE_OBJDUMP} -CS      ${elf_file} > ${lss_file}
		COMMAND ${CMAKE_OBJDUMP} -Ctx     ${elf_file} > ${dmp_file}
		COMMAND ${CMAKE_SIZE}    -B       ${elf_file}
		USES_TERMINAL
		VERBATIM
	)

endfunction()
