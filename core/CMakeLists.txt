add_library(plaid)

# 设置是否启用着色器 DSL
option(PLAID_SHADER_DSL "whether the dsl for shader shouble be enabled" ON)

# 公共头文件
target_include_directories(plaid PUBLIC include)

# 源码
aux_source_directory(src PLAID_SRC)
target_sources(plaid PRIVATE ${PLAID_SRC})

# 用宏指示着色器 DSL 开闭
message(STATUS "PLAID_SHADER_DSL=${PLAID_SHADER_DSL}")
if(PLAID_SHADER_DSL)
    target_compile_definitions(plaid PUBLIC -DPLAID_SHADER_DSL)
endif()
