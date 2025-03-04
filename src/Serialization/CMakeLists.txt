target_sources(${MAIN_TARGET}
	PRIVATE
	SceneSerialization.h
	SceneSerialization.cpp
)

target_include_directories(${MAIN_TARGET}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
