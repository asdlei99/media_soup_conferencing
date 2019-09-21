cmake_minimum_required(VERSION 3.5)

set(SIGNALLER_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(JSON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/json_parser/include)
set(JSON_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/json_parser/)


add_subdirectory(signaller)
add_subdirectory(json_parser)
add_subdirectory(mediasoup_conference)
add_subdirectory(websocket_server)
add_subdirectory(function_thread_handler)
#add_subdirectory(media_receiver)
add_subdirectory(video_render)
#add_subdirectory(media_render_util)
add_subdirectory(server_communication_util)
add_subdirectory(windows_util)
add_subdirectory(video_receiver_helper)