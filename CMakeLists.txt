project(httpsrv)
cmake_minimum_required(VERSION 2.8)

include_directories(.)

aux_source_directory(server/ server_srcs)
add_executable(httpsrv ${server_srcs} utility.cc)

aux_source_directory(client/ client_srcs)
add_executable(clitest ${client_srcs} utility.cc)

target_link_libraries(httpsrv pthread)
target_link_libraries(clitest pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
