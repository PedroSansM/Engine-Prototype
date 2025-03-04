add_library(glad STATIC "")

target_sources(glad
	PRIVATE
	src/glad.c
	include/glad/glad.h
	include/KHR/khrplatform.h
)

target_include_directories(glad
	PUBLIC
	include
)

#if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#find_path(LIBDL_INCLUDE_DIR dlfcn.h)
#find_library(LIBDL_LIBRARY libdl.so)
#target_include_directories(glad
#	PUBLIC
#	${LIBDL_INCLUDE_DIR}
#	
#)
#target_link_libraries(glad
#	PUBLIC
#	${LIBDL_LIBRARY}
#)
#ndif()
