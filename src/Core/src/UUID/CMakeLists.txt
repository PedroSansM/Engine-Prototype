target_sources(DommusCore
	PRIVATE
	UUID.cpp
	UUID.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	find_path(LIBUUID_INCLUDE_DIR uuid.h PATH_SUFFIXES uuid)
	find_library(LIBUUID_LIBRARY libuuid.a)
	target_include_directories(DommusCore
		PRIVATE
		${LIBUUID_INCLUDE_DIR}
	)
	target_link_libraries(DommusCore
		PRIVATE 
		${LIBUUID_LIBRARY}
	)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	#find_library(NAME_SERVICE Rpcns4.lib)
	#find_library(RUN_TIME Rpcrt4.lib)
	#target_link_libraries(DommusCore
	#	PRIVATE 
	#	${NAME_SERVICE}
	#	${RUN_TIME}
	#)
endif()
