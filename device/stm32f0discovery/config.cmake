include_guard(GLOBAL)

find_package(cmsis REQUIRED)
target_link_libraries(device INTERFACE cmsis)
add_library(device::${TARGET_PLATFORM} ALIAS device)

target_include_directories(device
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)
