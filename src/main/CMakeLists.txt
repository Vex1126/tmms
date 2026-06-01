aux_source_directory(. SRCS)
add_executable(tmms ${SRCS})
install(TARGETS tmms RUNTIME DESTINATION sbin)

# 添加这一行：链接 base 库
target_link_libraries(tmms base)

# 安装目标
install(TARGETS tmms RUNTIME DESTINATION sbin)