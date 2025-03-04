set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(DommusCore
	PRIVATE
	${INC_DIR}/fmod.hpp
	${INC_DIR}/fmod_codec.h
	${INC_DIR}/fmod_common.h
	${INC_DIR}/fmod_dsp_effects.h
	${INC_DIR}/fmod_errors.h
	${INC_DIR}/fmod_output.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(DommusCore
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/lib/x86_64/libfmod.so
)
