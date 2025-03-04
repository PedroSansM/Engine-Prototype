target_sources(DommusCore
	PRIVATE
	DebugDrawCommand.h
	Runtime.cpp
	Runtime.h
	UserData.h
	SceneLoader.cpp
	SceneLoader.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
