include_guard(GLOBAL)

target_link_libraries(startup
	INTERFACE
	--config ${CMAKE_CURRENT_LIST_DIR}/script.icf
)
