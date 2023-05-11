cmake_minimum_required()

project(plaid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS Off)

add_compile_options(-D_CRT_SECURE_NO_WARNINGS)

add_subdirectory(core)
add_subdirectory(json)
add_subdirectory(viewer)
