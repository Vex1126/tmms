cmake_minimum_required(VERSION 3.10)
project(base)

# 当前目录的源文件
aux_source_directory(. BASE_SRCS)

# 创建静态库
add_library(base STATIC ${BASE_SRCS})

# 设置包含路径
target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/include
)

# 关键：链接 jsoncpp 库
target_link_libraries(base
    ${CMAKE_BINARY_DIR}/json-prefix/lib/libjsoncpp.a
)