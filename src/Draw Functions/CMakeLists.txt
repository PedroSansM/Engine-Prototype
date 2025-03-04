target_sources(${MAIN_TARGET}
	PRIVATE
	Draw.cpp
	Draw.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
