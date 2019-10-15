cmake_minimum_required(VERSION 3.7)

add_definitions(/ZI)

add_library(mediasoup_conference
	mediasoup_conference.h
	mediasoup_conference.cpp
	room.h
#	util.h
#	util.cpp
	room_impl.h
	room_impl.cpp
	conference_handler.h 
	conference_handler.cpp
	peer_connection/peerConnectionUtils.cpp
	peer_connection/peerConnectionUtils.hpp
	peer_connection/VcmCapturer.cpp 
	peer_connection/VcmCapturer.hpp
	peer_connection/VideoCapturer.cpp
	peer_connection/VideoCapturer.hpp
	signalling/media_soup_conferencing_signalling.cpp 
	signalling/media_soup_conferencing_signalling.h
	)

	# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	
	target_compile_definitions(mediasoup_conference PUBLIC
	NOMINMAX
)
endif()

target_include_directories(mediasoup_conference PUBLIC 
	#$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
	${PROJECT_SOURCE_DIR}/include
	"${PROJECT_SOURCE_DIR}/deps/libsdptransform/include"
	"${PROJECT_SOURCE_DIR}/lib/signaller/"
	"${PROJECT_SOURCE_DIR}/lib/json_parser/"
	"${LIBWEBRTC_INCLUDE_PATH}"
	)


	# Compile definitions for libwebrtc.
target_compile_definitions(mediasoup_conference PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)

