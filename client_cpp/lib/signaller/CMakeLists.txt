cmake_minimum_required(VERSION 3.7)


add_library(signaller
	signaller.h
	signaller.cpp
	websocket_signaller.h
	websocket_signaller.cpp
	websocket_client_async.hpp
	)

target_include_directories(signaller PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	)
