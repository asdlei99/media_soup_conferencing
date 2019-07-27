cmake_minimum_required(VERSION 3.5)

set(EXE_NAME rendering_window)

add_executable(${EXE_NAME} WIN32
main.cpp
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

#[==[

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
	mediasoup_conference
	func_thread_handler
	websocket_server
	video_receiver
#amstrmid.lib
#crypt32.lib
#iphlpapi.lib

Strmiids
)
#]==]


