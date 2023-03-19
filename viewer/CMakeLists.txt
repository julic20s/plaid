add_executable(plaid_viewer)

# 源码
aux_source_directory(src PLAID_VIEWER_SRC)
aux_source_directory(src/data PLAID_VIEWER_DATA_SRC)
aux_source_directory(src/gltf PLAID_VIEWER_GLTF_SRC)
aux_source_directory(src/platform PLAID_VIEWER_PLATFORM_SRC)
aux_source_directory(src/scene PLAID_VIEWER_SCENE_SRC)
target_sources(
    plaid_viewer PRIVATE
    ${PLAID_VIEWER_SRC}
    ${PLAID_VIEWER_DATA_SRC}
    ${PLAID_VIEWER_GLTF_SRC}
    ${PLAID_VIEWER_PLATFORM_SRC}
    ${PLAID_VIEWER_SCENE_SRC}
)

# 依赖
target_link_libraries(plaid_viewer plaid)
target_link_libraries(plaid_viewer plaid_json)

# 为 WIN32 平台设置专属宏
message(STATUS "PLAID_VIEWER_WIN32=${WIN32}")
if (WIN32)
    target_compile_definitions(plaid PUBLIC -DPLAID_VIEWER_WIN32)
endif()
