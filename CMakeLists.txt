cmake_minimum_required(VERSION 3.25.1)

project(plaid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS Off)

add_subdirectory(core)
add_subdirectory(viewer)
