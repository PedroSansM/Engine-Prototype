target_sources(DommusCore
	PRIVATE
	DCoreAssert.h
	ReturnError.h
	ReturnError.cpp
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
