target_sources(DommusCore
	PRIVATE
	ReadWriteLockGuard.cpp
	ReadWriteLockGuard.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
