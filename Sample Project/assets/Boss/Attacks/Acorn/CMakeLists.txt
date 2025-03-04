target_sources(${MAIN_TARGET}
	PUBLIC
	AcornComponent.cpp
	AcornComponent.h
	AcornManagerComponent.cpp
	AcornManagerComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
