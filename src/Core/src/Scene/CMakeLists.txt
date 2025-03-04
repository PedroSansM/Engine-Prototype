target_sources(DommusCore
	PRIVATE
	Scene.cpp
	Scene.h
	SceneTypes.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
