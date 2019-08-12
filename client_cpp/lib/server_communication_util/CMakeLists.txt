cmake_minimum_required(VERSION 3.7)


add_library(server_communication_util
	util.h
	util.cpp
	rendering_server_client.h
	rendering_server_client.cpp
	)

	# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

endif()

target_include_directories(server_communication_util PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
	${PROJECT_SOURCE_DIR}/include
	"${PROJECT_SOURCE_DIR}/deps/libsdptransform/include"
	"${PROJECT_SOURCE_DIR}/lib/signaller/"
	"${PROJECT_SOURCE_DIR}/lib/json_parser/"
	)
