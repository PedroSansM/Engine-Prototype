target_sources(DommusCore
	PRIVATE
	Graphics.h
	TemplateUtils.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(Animation)
add_subdirectory(Attribute\ Name)
add_subdirectory(Asset\ Manager)
add_subdirectory(ECS)
add_subdirectory(Data\ Structure)
add_subdirectory(Profiling)
add_subdirectory(Serialization)
add_subdirectory(Native\ Scripting)
add_subdirectory(Component)
add_subdirectory(Builtin\ Component)
add_subdirectory(Scene)
add_subdirectory(Error)
add_subdirectory(Input)
add_subdirectory(Async)
add_subdirectory(UUID)
add_subdirectory(Reference)
add_subdirectory(Rendering)
add_subdirectory(Math)
add_subdirectory(Runtime)
add_subdirectory(Thread)
add_subdirectory(Physics)
add_subdirectory(Global\ Configuration)
add_subdirectory(Sound)
