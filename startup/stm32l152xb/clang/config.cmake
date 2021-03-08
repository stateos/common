include_guard(GLOBAL)

target_sources(startup
	INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/startup.s
)

target_link_libraries(startup
	INTERFACE
	--scatter=${CMAKE_CURRENT_LIST_DIR}/script.sct
)
