set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(DommusCore
	PRIVATE
	${INC_DIR}/fsbank.h
	${INC_DIR}/fsbank_errors.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(DommusCore
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/fsbank_vc.lib
)
