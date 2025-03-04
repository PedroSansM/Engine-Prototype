cmake_minimum_required(VERSION 3.28)

project(${MAIN_TARGET})
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${MAIN_TARGET} "")

set (EDITOR_ENV ON)

add_subdirectory(src)
add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/ImGuizmo)

if (${PROJECT_PATH_DEFINED})
	add_subdirectory(${PROJECT_PATH} ${PROJECT_PATH}/output)
endif()

target_link_libraries(${MAIN_TARGET} PRIVATE DommusCore)
target_link_libraries(${MAIN_TARGET} PRIVATE yaml-cpp::yaml-cpp)

set (MAKE_SCENE_TEST OFF)
set (SHOW_HIERARCHY_IN_CONSOLE_TEST OFF)
set (SCENE_HIERARCHY_PANEL_TEST OFF)
set (TEST_RECICLING_VECTOR OFF)
set (TEST_STRING OFF)
set (ECS_TEST OFF)
set (ITYPE_LIST_TEST OFF)
set (CVECTOR_TEST OFF)
set (NEW_ECS_TEST OFF)

target_sources(${MAIN_TARGET}
	PRIVATE
	src/EntryPoint.cpp)
target_compile_definitions(${MAIN_TARGET}
	PRIVATE
	EDITOR)
 
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_definitions(${MAIN_TARGET}
		PRIVATE
		UNIX_OS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	STRING(REPLACE "/" "\\" BUILD_DIRECTORY ${CMAKE_BINARY_DIR})
	add_custom_command(TARGET ${MAIN_TARGET} POST_BUILD
		COMMAND xcopy ${EDITOR_PATH}\\src\\Core\\vendor\\fmodstudio\\Windows\\api\\core\\lib\\x64 ${BUILD_DIRECTORY} /c /y
		COMMAND xcopy ${EDITOR_PATH}\\src\\Core\\vendor\\fmodstudio\\Windows\\api\\fsbank\\lib\\x64 ${BUILD_DIRECTORY} /c /y
		COMMAND xcopy ${EDITOR_PATH}\\src\\Core\\vendor\\fmodstudio\\Windows\\api\\studio\\lib\\x64 ${BUILD_DIRECTORY} /c /y
	)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	target_compile_definitions(${MAIN_TARGET}
		PRIVATE
		NDEBUG)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	target_compile_definitions(${MAIN_TARGET}
		PRIVATE
		DEBUG)
#	target_compile_options(${MAIN_TARGET}
#		PRIVATE
#		-fsanitize=thread -fnon-call-exceptions)
#	target_link_options(${MAIN_TARGET}
#		PRIVATE
#		-fsanitize=thread)
#	target_link_options(${MAIN_TARGET}
#		PRIVATE
#		-fsanitize=address)
endif()
