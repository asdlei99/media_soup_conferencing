cmake_minimum_required(VERSION 3.5)

set(EXE_NAME app)

add_executable(${EXE_NAME} 
main.cxx
util.h
util.cpp
media_soup_conferencing_signalling.h
media_soup_conferencing_signalling.cpp
peer_connection/peerConnectionUtils.hpp
peer_connection/peerConnectionUtils.cpp
peer_connection/VcmCapturer.cpp
peer_connection/VcmCapturer.hpp
peer_connection/VideoCapturer.hpp
peer_connection/VideoCapturer.cpp
)
# Add some compile flags to our source files.
if(MSVC)
	#set_source_files_properties(${SOURCE_FILES}
	#	PROPERTIES COMPILE_FLAGS "/W3 /WX")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	
	target_compile_definitions(${EXE_NAME} PUBLIC
	NOMINMAX
)
else()
	set_source_files_properties(${SOURCE_FILES}
		PROPERTIES COMPILE_FLAGS -Wall -Wextra -Wpedantic)
endif()

# Private (implementation) header files.
target_include_directories(${EXE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Public (interface) headers from dependencies.
target_include_directories(${EXE_NAME} PUBLIC
	"${PROJECT_SOURCE_DIR}/deps/libsdptransform/include"
	"${PROJECT_SOURCE_DIR}/lib/signaller/"
	"${PROJECT_SOURCE_DIR}/lib/json_parser/"
	"${LIBWEBRTC_INCLUDE_PATH}"
	"${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp"
)

target_link_directories( ${EXE_NAME} PUBLIC
	$<BUILD_INTERFACE:${BOOST_LIB}/stage/lib>
)

# Public (interface) dependencies.
target_link_libraries(${EXE_NAME} PUBLIC
#	sdptransform
	${LIBWEBRTC_BINARY_PATH}/webrtc${CMAKE_STATIC_LIBRARY_SUFFIX}
	${PROJECT_SOURCE_DIR}/lib/mediasoup/Debug/mediasoupclient.lib
	${PROJECT_SOURCE_DIR}/lib/mediasoup/Debug/sdptransform.lib
	signaller
	json_parser
	winmm
	dmoguids
	wmcodecdspuuid
	msdmo
	secur32
#amstrmid.lib
#crypt32.lib
#iphlpapi.lib

Strmiids
)

# Compile definitions for libwebrtc.
target_compile_definitions(${EXE_NAME} PUBLIC
	$<$<NOT:$<PLATFORM_ID:Windows>>:WEBRTC_POSIX>
	$<$<PLATFORM_ID:Windows>:WEBRTC_WIN>
	$<$<PLATFORM_ID:Darwin>:WEBRTC_MAC>
)


