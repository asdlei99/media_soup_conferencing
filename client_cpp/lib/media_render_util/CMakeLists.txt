cmake_minimum_required(VERSION 3.7)

add_library(media_render_util
	video_render_util.h
	video_render_util.cpp
	)

target_include_directories(media_render_util PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
	)
