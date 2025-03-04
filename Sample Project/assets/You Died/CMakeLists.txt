target_sources(${MAIN_TARGET}
	PUBLIC
	YouDiedMessageComponent.cpp
	YouDiedMessageComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
