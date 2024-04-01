# 收集当前文件所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)


# 源文件编译生成可执行文件
add_executable(chatserver ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

# 可执行文件依赖的链接库文件
target_link_libraries(
    chatserver muduo_net muduo_base mysqlclient hiredis pthread)