target_sources(${MAIN_TARGET}
	PRIVATE
	SpriteSheetGen.cpp
	SpriteSheetGen.h
)

target_include_directories(${MAIN_TARGET}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
