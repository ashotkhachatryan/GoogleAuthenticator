cmake_minimum_required(VERSION 3.1)

project(google_authenticator)

add_subdirectory(src)

option(BUILD_EXECUTABLE "Build the example app" OFF)

if (BUILD_EXECUTABLE)
    set(CMAKE_CXX_STANDARD 20)
    add_executable(main app/main.cpp)
    target_include_directories(main PRIVATE include)
    target_link_libraries(main PUBLIC google_authenticator)
endif()
