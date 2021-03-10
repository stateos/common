cmake_minimum_required(VERSION 3.18)

include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 11)

set(TOOLCHAIN_gcc      "")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_gcc}avr-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_gcc}avr-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_gcc}avr-gcc")
set(CMAKE_AR           "${TOOLCHAIN_gcc}avr-ar")
set(CMAKE_SIZE         "${TOOLCHAIN_gcc}avr-size")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_gcc}avr-objcopy")
set(CMAKE_OBJDUMP      "${TOOLCHAIN_gcc}avr-objdump")
set(CMAKE_NM           "${TOOLCHAIN_gcc}avr-nm")
set(CMAKE_STRIP        "${TOOLCHAIN_gcc}avr-strip")
set(CMAKE_RANLIB       "${TOOLCHAIN_gcc}avr-ranlib")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release MinSizeRel)

set(INIT_FLAGS             "-mmcu=${TARGET_DEVICE} -fdata-sections -ffunction-sections -mno-gas-isr-prologues")

set(CMAKE_C_FLAGS          "${INIT_FLAGS}"                       CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS        "${INIT_FLAGS} -fno-use-cxa-atexit"   CACHE INTERNAL "cxx compiler flags")
set(CMAKE_ASM_FLAGS        "${INIT_FLAGS} -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -demangle"         CACHE INTERNAL "exe linker flags")

set(CMAKE_C_FLAGS_DEBUG            "-g -ggdb -Og"       CACHE INTERNAL "c debug compiler flags")
set(CMAKE_CXX_FLAGS_DEBUG          "-g -ggdb -Og"       CACHE INTERNAL "cxx debug compiler flags")
set(CMAKE_ASM_FLAGS_DEBUG          "-g -ggdb"           CACHE INTERNAL "asm debug compiler flags")

set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-DNDEBUG -g -O"     CACHE INTERNAL "c release with debug info compiler flags")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DNDEBUG -g -O"     CACHE INTERNAL "cxx release with debug info compiler flags")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-DNDEBUG -g"        CACHE INTERNAL "asm release with debug info compiler flags")

set(CMAKE_C_FLAGS_RELEASE          "-DNDEBUG -O"        CACHE INTERNAL "c release compiler flags")
set(CMAKE_CXX_FLAGS_RELEASE        "-DNDEBUG -O"        CACHE INTERNAL "cxx release compiler flags")
set(CMAKE_ASM_FLAGS_RELEASE        "-DNDEBUG"           CACHE INTERNAL "asm release compiler flags")

set(CMAKE_C_FLAGS_MINSIZEREL       "-DNDEBUG -flto -Os" CACHE INTERNAL "c min size release compiler flags")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-DNDEBUG -flto -Os" CACHE INTERNAL "cxx min size release compiler flags")
set(CMAKE_ASM_FLAGS_MINSIZEREL     "-DNDEBUG -flto"     CACHE INTERNAL "asm min size release compiler flags")

set(STD_WARNING_FLAGS -Wall)
set(ALL_WARNING_FLAGS ${STD_WARNING_FLAGS} -Wextra -Wpedantic -Wconversion -Wshadow -Wlogical-op)

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

	if (exceptions IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using exceptions")
	else()
		add_flags(CMAKE_CXX_FLAGS -fno-rtti -fno-exceptions)
	endif()

	if (lto IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using lto")
		target_compile_options(${target} PRIVATE -flto)
	endif()

	if (nowarnings IN_LIST ARGN)
		message(STATUS "${STARTMSG} Using nowarnings")
		add_flags(CMAKE_C_FLAGS   ${STD_WARNING_FLAGS})
		add_flags(CMAKE_CXX_FLAGS ${STD_WARNING_FLAGS})
		add_flags(CMAKE_ASM_FLAGS ${STD_WARNING_FLAGS})
	else()
		add_flags(CMAKE_C_FLAGS   ${ALL_WARNING_FLAGS})
		add_flags(CMAKE_CXX_FLAGS ${ALL_WARNING_FLAGS} -Wzero-as-null-pointer-constant)
		add_flags(CMAKE_ASM_FLAGS ${ALL_WARNING_FLAGS})
	endif()

	if (CMAKE_SYSTEM_NAME STREQUAL "Generic")
		set_target_properties(${target} PROPERTIES SUFFIX ".elf")
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

	target_link_libraries(${target}
		PRIVATE
		-Wl,-Map=${map_file}
	)

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
