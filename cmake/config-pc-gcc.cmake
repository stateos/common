include_guard(GLOBAL)

set(TARGET_PLATFORM "pc")
set(TARGET_CHIP     "amd64")
set(TARGET_DEVICE   "x86")
set(TARGET_CORE     "x86")
set(TARGET_ARCH     "x86")
set(TARGET_COMPILER "gcc")

include("${CMAKE_CURRENT_LIST_DIR}/toolchain-x86-gcc.cmake")

function(setup_target target)

	setup_target_compiler(${target} ${ARGN})

endfunction()
