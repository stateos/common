include_guard(GLOBAL)

target_link_libraries(startup
	INTERFACE
	-T${CMAKE_CURRENT_LIST_DIR}/script.ld
)
