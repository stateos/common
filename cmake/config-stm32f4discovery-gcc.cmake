include_guard(GLOBAL)

set(TARGET_PLATFORM "stm32f4discovery")
set(TARGET_CHIP     "stm32f407vg")
set(TARGET_DEVICE   "stm32f4")
set(TARGET_CORE     "cortex-m4")
set(TARGET_ARCH     "cortexm")
set(TARGET_COMPILER "gnucc")

include("${CMAKE_CURRENT_LIST_DIR}/toolchain-cortexm-gcc.cmake")

function(setup_target target)

	set(elf_file ${target}.elf)
	set(hex_file ${target}.hex)

	set(OOCD_INIT -d0 -f board/stm32f4discovery.cfg -c init -c targets)
	set(OOCD_SAVE -c "reset halt" -c "flash write_image erase ${elf_file}" -c "verify_image ${elf_file}")
	set(OOCD_HOST -c "arm semihosting enable")
	set(OOCD_EXEC -c "reset run")
	set(OOCD_EXIT -c shutdown)

	find_program(qemuarm "qemu-system-gnuarmeclipse")
	find_program(openocd "openocd")
	find_program(stlink  "st-link_cli")

	if (${__FLASH})
		message(STATUS "${STARTMSG} Configuring for FLASH function")
		unset(__FLASH CACHE)
		set(__SEMIHOST OFF)
		add_custom_target(${target}-custom
			ALL
		#	COMMAND ${stlink} -Q -c SWD UR -P ${hex_file} -V -Rst || true
			COMMAND ${openocd} ${OOCD_INIT} ${OOCD_SAVE} ${OOCD_EXEC} ${OOCD_EXIT} || true
			DEPENDS ${target}
			COMMENT "Flashing the ${TARGET_PLATFORM} device"
			USES_TERMINAL
			VERBATIM
		)
	elseif (${__QEMU})
		message(STATUS "${STARTMSG} Configuring for QEMU function")
		unset(__QEMU CACHE)
		set(__NOFPU ON)
		add_custom_target(${target}-custom
			ALL
			COMMAND ${qemuarm} -semihosting -board STM32F4-Discovery -image ${elf_file} || true
			DEPENDS ${target}
			COMMENT "Simulating the ${TARGET_PLATFORM} device"
			USES_TERMINAL
			VERBATIM
		)
	elseif (${__MONITOR})
		message(STATUS "${STARTMSG} Configuring for MONITOR function")
		unset(__MONITOR CACHE)
		set(__SEMIHOST ON)
		add_custom_target(${target}-custom
			ALL
			COMMAND ${openocd} ${OOCD_INIT} ${OOCD_SAVE} ${OOCD_HOST} ${OOCD_EXEC} || true
			DEPENDS ${target}
			COMMENT "Monitoring the ${TARGET_PLATFORM} device"
			USES_TERMINAL
			VERBATIM
		)
	endif()

	if ((nofpu IN_LIST ARGN) OR (${__NOFPU}))
		message(STATUS "${STARTMSG} Using software floats")
		set(FPU_FLAGS -mfpu=fpv4-sp-d16 -mfloat-abi=soft -ffast-math)
	else()
		message(STATUS "${STARTMSG} Using hardware floats")
		set(FPU_FLAGS -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffast-math)
	endif()

	update_flags(CMAKE_ASM_FLAGS ${FPU_FLAGS})
	update_flags(CMAKE_C_FLAGS   ${FPU_FLAGS})
	update_flags(CMAKE_CXX_FLAGS ${FPU_FLAGS})
	target_link_options(${target} PRIVATE ${FPU_FLAGS})

	target_compile_definitions(test PRIVATE STM32F407xx)

	setup_target_compiler(${target} ${ARGN})

endfunction()
