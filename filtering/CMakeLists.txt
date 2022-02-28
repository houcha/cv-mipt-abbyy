cmake_minimum_required(VERSION 2.8)

project( Filtering )

set(CMAKE_CXX_FLAGS "-O3 -Wall")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( Filtering main.cpp )
target_link_libraries( Filtering ${OpenCV_LIBS} )

