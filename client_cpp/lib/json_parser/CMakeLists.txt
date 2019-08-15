cmake_minimum_required(VERSION 3.7)

set(LIBWEBRTC_INCLUDE_PATH "D:/webrtcBuild/Sept/webrtc_checkout/src")
set(ABSL_PATH "${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp")

add_library(json_parser
	json_parser.h
	json_parser.cpp
	)

target_include_directories(json_parser PUBLIC 
	$<BUILD_INTERFACE:${ABSL_PATH}>
	${PROJECT_SOURCE_DIR}/include
	)

target_sources(json_parser PRIVATE
${ABSL_PATH}/absl/types/bad_any_cast.cc
)

