target_sources(DommusCore
	PRIVATE
	EntityQuery.cpp
	EntityQuery.h
	EntityRef.cpp
	EntityRef.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
