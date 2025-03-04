target_sources(DommusCore
	PRIVATE
	GlobalConfig.h
	AspectRatioUtilities.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
