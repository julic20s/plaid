add_library(plaid_json)

# 公共头文件
target_include_directories(plaid_json PUBLIC include)

# 源码
aux_source_directory(src PLAID_JSON_SRC)
target_sources(plaid_json PRIVATE ${PLAID_JSON_SRC})



