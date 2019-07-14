cmake_minimum_required(VERSION 3.7)


add_library(websocket_server
	websocket_server.h
		websocket_server.cpp
	)

target_include_directories(websocket_server PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
	)
