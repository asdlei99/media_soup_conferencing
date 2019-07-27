cmake_minimum_required(VERSION 3.7)

add_library(video_render
	renderer.h
	renderer.cpp
	dirextx_render.h
	dirextx_render.cpp
	)

target_include_directories(video_render PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	)
	#[==[
set_target_properties(video_render PROPERTIES 
	CXX_STANDARD 17
	)
	#]==]