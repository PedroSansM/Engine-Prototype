target_sources(${MAIN_TARGET}
	PRIVATE
	GlobalConfigurationSerializer.cpp
	GlobalConfigurationSerializer.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
