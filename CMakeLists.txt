cmake_minimum_required()

project(plaid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS Off)

add_subdirectory(core)
add_subdirectory(gltf)
add_subdirectory(viewer)
