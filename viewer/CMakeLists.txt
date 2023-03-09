add_executable(plaid_viewer)

# 源码
aux_source_directory(src PLAID_VIEWER_SRC)
target_sources(plaid_viewer PRIVATE ${PLAID_VIEWER_SRC})

# 依赖
target_link_libraries(plaid_viewer plaid)

# 为 WIN32 平台设置专属宏
message(STATUS "PLAID_VIEWER_WIN32=${WIN32}")
if (WIN32)
    target_compile_definitions(plaid PUBLIC -DPLAID_VIEWER_WIN32)
endif()
