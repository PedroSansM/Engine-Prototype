add_library(DommusCore STATIC)

target_sources(DommusCore
	PRIVATE
	DommusCore.h
)

add_subdirectory(vendor/Box2D/src)
add_subdirectory(vendor/fmodstudio)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/stb)

target_include_directories(DommusCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DommusCore PUBLIC glm::glm-header-only)
target_link_libraries(DommusCore PUBLIC glfw)
target_link_libraries(DommusCore PUBLIC glad)
target_link_libraries(DommusCore PUBLIC box2d::box2d)

if (EDITOR_ENV)
	target_compile_definitions(DommusCore
		PUBLIC
		EDITOR
	)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	target_compile_definitions(DommusCore
		PUBLIC
		NDEBUG
	)
endif()

target_compile_definitions(DommusCore
	PUBLIC
	GLM_ENABLE_EXPERIMENTAL
)

add_subdirectory(src)
