target_sources(DommusCore
	PRIVATE
	DCoreMath.cpp
	DCoreMath.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
