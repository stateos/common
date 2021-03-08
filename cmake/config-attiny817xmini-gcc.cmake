include_guard(GLOBAL)

set(TARGET_PLATFORM "attiny817xmini")
set(TARGET_CHIP     "attiny817")
set(TARGET_DEVICE   "attiny817")
set(TARGET_CORE     "attiny")
set(TARGET_ARCH     "avr8")
set(TARGET_COMPILER "gcc")

include("${CMAKE_CURRENT_LIST_DIR}/toolchain-avr8-gcc.cmake")

function(setup_target target)

	set(elf_file ${target}.elf)

	find_program(program "atprogram")

	if (${__FLASH})
		message(STATUS "${STARTMSG} Configuring for FLASH function")
		unset(__FLASH CACHE)
		add_custom_target(${target}-custom
			ALL
			COMMAND ${program} -t medbg -i updi -d ${TARGET_DEVICE} program -c -fl -f ${elf_file} --verify || true
			DEPENDS ${target}
			COMMENT "Flashing the ${TARGET_PLATFORM} device"
			USES_TERMINAL
			VERBATIM
		)
	endif()

	target_compile_definitions(test
		PRIVATE
		STM32L152xB
	)

	target_compile_definitions(test
		PRIVATE
		F_CPU=20000000
	)

	setup_target_compiler(${target} ${ARGN})

endfunction()
