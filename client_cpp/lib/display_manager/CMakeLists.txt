cmake_minimum_required(VERSION 3.7)

add_library(display_manager
	layout_manager.h
	layout_manager.cpp
	win32_window.h
	win32_window.cpp
	win32_window_creator.h
	win32_window_creator.cpp
	window.h
	window_creater.h
	)


if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	

endif()

target_include_directories(display_manager PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	)