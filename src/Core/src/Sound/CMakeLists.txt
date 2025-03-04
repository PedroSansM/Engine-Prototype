target_sources(DommusCore
	PRIVATE
	Sound.cpp
	Sound.h
	SoundEventInstance.cpp
	SoundEventInstance.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
