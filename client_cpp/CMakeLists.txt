cmake_minimum_required(VERSION 3.5)
project(mediasoup_conference LANGUAGES CXX)
set(BOOST_LIB "D:/boost_1_69_0/boost_1_69_0")

set(ABSL_PATH "${LIBWEBRTC_INCLUDE_PATH}/third_party/abseil-cpp")

ADD_DEFINITIONS(-D_UNICODE)
add_definitions(/ZI)

if(MSVC)
set(CMAKE_CXX_FLAGS_RELEASE "/MT")
set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
endif(MSVC)
add_subdirectory(app)
add_subdirectory(lib)
add_subdirectory(conference_app)
add_subdirectory(rendering_window)
add_subdirectory(unit_test)