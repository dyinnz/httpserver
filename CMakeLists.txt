project(httpsrv)
cmake_minimum_required(VERSION 2.8)

include_directories(.)

aux_source_directory(server/ server_srcs)
add_executable(httpsrv ${server_srcs})

aux_source_directory(client/ client_srcs)
add_executable(clitest ${client_srcs})

target_link_libraries(httpsrv pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
