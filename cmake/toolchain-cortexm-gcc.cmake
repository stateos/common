cmake_minimum_required(VERSION 3.18)

include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ${TARGET_CORE})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

set(TOOLCHAIN_gcc      "")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_gcc}arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_gcc}arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_gcc}arm-none-eabi-gcc")
set(CMAKE_AR           "${TOOLCHAIN_gcc}arm-none-eabi-ar")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_gcc}arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_gcc}arm-none-eabi-objdump")
set(CMAKE_SIZE         "${TOOLCHAIN_gcc}arm-none-eabi-size")
set(CMAKE_NM           "${TOOLCHAIN_gcc}arm-none-eabi-nm")
set(CMAKE_STRIP        "${TOOLCHAIN_gcc}arm-none-eabi-strip")
set(CMAKE_RANLIB       "${TOOLCHAIN_gcc}arm-none-eabi-ranlib")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(COMMON_OPTIONS -mthumb -mcpu=${TARGET_CORE} -fdata-sections -ffunction-sections)

add_compile_options(${COMMON_OPTIONS} $<$<COMPILE_LANGUAGE:ASM>:-xassembler-with-cpp> $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)
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

	add_compile_options($<$<COMPILE_LANGUAGE:ASM>:"-x assembler-with-cpp"> $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)

	if (nostartfiles IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using nostartfiles")
		target_link_options(${target} PRIVATE -nostartfiles)
	endif()

	if (exceptions IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using exceptions")
	else()
		target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
	endif()

	if (stdlib IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using stdlib")
	else()
		message(STATUS "${STARTMSG} Using nanolib")
		target_link_options(${target} PRIVATE -specs=nano.specs)
	endif()

	if (nosys IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using nosys")
		add_definitions("-D__NOSYS")
		target_link_options(${target} PRIVATE -specs=nosys.specs)
	endif()

	if ((semihost IN_LIST ARGN) OR (${__SEMIHOST}))
		message(STATUS "${STARTMSG} Using semihosting")
		add_definitions("-D__SEMIHOST")
		target_link_options(${target} PRIVATE -specs=rdimon.specs)
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

	if (CMAKE_SYSTEM_NAME STREQUAL "Generic")
		set_target_properties(${target} PROPERTIES SUFFIX .elf)
	endif()

	if(CMAKE_GENERATOR STREQUAL "Ninja")
		target_compile_options(${target} PRIVATE -fdiagnostics-color)
	endif()

	set(elf_file ${target}.elf)
	set(map_file ${target}.map)
	set(hex_file ${target}.hex)
	set(bin_file ${target}.bin)
	set(lss_file ${target}.lss)
	set(dmp_file ${target}.dmp)

	target_link_options(${target} PRIVATE -Wl,-Map=${map_file})

	add_custom_command(
		TARGET  ${target}
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -Oihex   ${elf_file}   ${hex_file}
		COMMAND ${CMAKE_OBJCOPY} -Obinary ${elf_file}   ${bin_file}
		COMMAND ${CMAKE_OBJDUMP} -CS      ${elf_file} > ${lss_file}
		COMMAND ${CMAKE_OBJDUMP} -Ctx     ${elf_file} > ${dmp_file}
		COMMAND ${CMAKE_SIZE}    -B       ${elf_file}
		USES_TERMINAL
		VERBATIM
	)

endfunction()
