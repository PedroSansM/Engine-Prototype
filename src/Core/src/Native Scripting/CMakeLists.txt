target_sources(DommusCore
	PRIVATE
	ScriptComponent.cpp
	ScriptComponent.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
