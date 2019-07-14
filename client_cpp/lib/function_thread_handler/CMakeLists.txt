cmake_minimum_required(VERSION 3.7)


add_library(func_thread_handler
	func_thread_handler.h
	func_thread_handler.cpp
	msg_loop_libraray.h
	)

target_include_directories(func_thread_handler PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
	)