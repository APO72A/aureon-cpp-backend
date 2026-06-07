cmake_minimum_required(VERSION 3.20)
project(aureon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(aureon
    src/main.cpp
    src/server/Platform.cpp
    src/server/HttpResponse.cpp
    src/server/HttpRequest.cpp
    src/server/Router.cpp
    src/server/HttpServer.cpp
)

if(WIN32)
    target_link_libraries(aureon ws2_32)
endif ()