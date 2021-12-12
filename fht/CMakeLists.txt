cmake_minimum_required(VERSION 2.8)

project( FHT )

set(CMAKE_CXX_FLAGS "-O0 -Wall -g")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( FHT main.cpp fht.cpp)
target_link_libraries( FHT ${OpenCV_LIBS} )
