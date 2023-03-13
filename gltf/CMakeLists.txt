add_library(plaid_gltf)

# 公共头文件
target_include_directories(plaid_gltf PUBLIC include)

# 源码
aux_source_directory(src PLAID_GLTF_SRC)
target_sources(plaid_gltf PRIVATE ${PLAID_GLTF_SRC})



