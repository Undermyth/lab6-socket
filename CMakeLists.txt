cmake_minimum_required(VERSION 3.21)

project(socketapp)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(${PROJECT_NAME} client.cpp server.cpp main.cpp)