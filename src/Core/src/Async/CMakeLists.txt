target_sources(DommusCore
	PRIVATE
	AsyncOperation.cpp
	AsyncOperation.h
	AsyncOperationRef.cpp
	AsyncOperationRef.h
	Promise.h
	PromiseRef.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(Threads)
target_link_libraries(DommusCore PRIVATE ${CMAKE_THREAD_LIBS_INIT})
