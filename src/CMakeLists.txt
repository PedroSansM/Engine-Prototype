target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(Config)
add_subdirectory(Editor\ Animation)
add_subdirectory(Editor\ Asset\ Management)
add_subdirectory(Core)
add_subdirectory(Panel)
add_subdirectory(Program\ Context)
add_subdirectory(File\ Browser)
add_subdirectory(Log)
add_subdirectory(Serialization)
add_subdirectory(Path)
add_subdirectory(Payload)
add_subdirectory(Draw\ Functions)
add_subdirectory(Type\ Names)
add_subdirectory(Window)
add_subdirectory(Sprite\ Sheet\ Gen)
