target_sources(${MAIN_TARGET}
	PUBLIC
	VenusComponent.cpp
	VenusComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

