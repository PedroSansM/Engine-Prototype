set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(DommusCore
	PRIVATE
	${INC_DIR}/fmod_studio.hpp
	${INC_DIR}/fmod_studio_common.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(DommusCore
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/fmodstudio_vc.lib
)
