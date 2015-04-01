project(httpsrv)
cmake_minimum_required(VERSION 3.0)

aux_source_directory(. dir_srcs)
add_executable(httpsrv ${dir_srcs})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
