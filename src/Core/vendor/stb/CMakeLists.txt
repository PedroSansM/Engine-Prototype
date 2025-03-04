target_sources(${MAIN_TARGET}
	PRIVATE
	stb_image.h
	stb_image_write.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
