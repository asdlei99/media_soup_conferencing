cmake_minimum_required(VERSION 3.7)

add_library(windows_util
	windows_util.h
	windows_util.cpp
	)

	#[==[
target_include_directories(media_render_util PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	#$<BUILD_INTERFACE:${ABSL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
	)

target_link_libraries(media_render_util PUBLIC
server_communication_util
)

#]==]
