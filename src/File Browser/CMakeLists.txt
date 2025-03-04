target_sources(${MAIN_TARGET}
	PRIVATE
	FileBrowser.h
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_sources(${MAIN_TARGET}
		PRIVATE
		UnixFileBrowser.cpp
	)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(${MAIN_TARGET}
		PRIVATE
		WindowsFileBrowser.cpp
	)
endif()

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
