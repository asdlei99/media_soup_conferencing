cmake_minimum_required(VERSION 3.7)


add_library(json_parser
	json_parser.h
	json_parser.cpp
	)

target_include_directories(json_parser PUBLIC 
	$<BUILD_INTERFACE:${BOOST_LIB}>
	$<BUILD_INTERFACE:${ABSL_PATH}>
	$<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/lib/>
	${PROJECT_SOURCE_DIR}/include
	"${PROJECT_SOURCE_DIR}/deps/libsdptransform/include"
	)

target_sources(json_parser PUBLIC
${ABSL_PATH}/absl/types/bad_any_cast.cc
)

