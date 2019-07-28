cmake_minimum_required(VERSION 3.7)


add_library(video_receiver
	video_track_receiver_impl.h
	video_track_receiver_impl.cpp
	video_frame_callback.h
	video_track_receiver.h
	)

	# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	
	target_compile_definitions(video_receiver PUBLIC
	NOMINMAX
)
endif()

target_include_directories(video_receiver PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
#	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
#	${PROJECT_SOURCE_DIR}/include
#	"${PROJECT_SOURCE_DIR}/deps/libsdptransform/include"
#	"${PROJECT_SOURCE_DIR}/lib/signaller/"
#	"${PROJECT_SOURCE_DIR}/lib/json_parser/"
	"${LIBWEBRTC_INCLUDE_PATH}"
	)


	# Compile definitions for libwebrtc.
target_compile_definitions(video_receiver PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)

