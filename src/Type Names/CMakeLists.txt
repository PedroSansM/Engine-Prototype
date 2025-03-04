target_sources(${MAIN_TARGET}
	PRIVATE
	TypeNames.cpp
	TypeNames.h
)

target_include_directories(${MAIN_TARGET}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
