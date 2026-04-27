cmake_minimum_required(VERSION 4.2)
project(cpp_backend_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp_backend_test main.cpp)

target_link_libraries(cpp_backend_test ws2_32)

