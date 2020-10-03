cmake_minimum_required(VERSION 3.17)
project(draft_http_tunnel)

set(CMAKE_CXX_STANDARD 20)
include_directories(dependencies/asio/asio/include)

add_executable(draft_http_tunnel main.cpp)
