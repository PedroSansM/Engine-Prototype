target_sources(${MAIN_TARGET}
	PRIVATE
	AnimationManager.cpp
	AnimationManager.h
	AnimationStateMachineManager.cpp
	AnimationStateMachineManager.h
	PhysicsMaterialManager.cpp
	PhysicsMaterialManager.h
	EditorAssetManager.cpp
	EditorAssetManager.h
	MaterialManager.cpp
	MaterialManager.h
	SceneManager.cpp
	SceneManager.h
	SpriteSheetGenManager.cpp
	SpriteSheetGenManager.h
	TextureManager.cpp
	TextureManager.h
	TextureMetadataManager.cpp
	TextureMetadataManager.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
