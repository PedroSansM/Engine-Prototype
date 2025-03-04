target_sources(DommusCore
	PRIVATE
	DebugRectObjectRenderer.h
	DebugRectObjectShader.cpp
	DebugRectObjectShader.h
	DebugShader.cpp
	DebugShader.h
	Material.h
	Quad.h
	Renderer.cpp
	Renderer.h
	RendererTypes.h
	Shaders.cpp
	Shaders.h
	SpriteMaterial.cpp
	SpriteMaterial.h
	Texture2D.cpp
	Texture2D.h
	TextureUnits.h
	UnlitSpriteMaterialShader.cpp
	UnlitSpriteMaterialShader.h
	UnlitTexturedObjectRenderer.h
	VertexStructures.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

#find_path(GLFW_INCLUDE_DIR glfw3.h PATH_SUFFIXES GLFW GLFW/include)
#find_library(GLFW_LIBRARY libglfw3.a)
#target_include_directories(DommusCore
#	PRIVATE
#	${GLFW_INCLUDE_DIR}
#)
#target_link_libraries(DommusCore
#	PRIVATE 
#	${GLFW_LIBRARY}
#)
