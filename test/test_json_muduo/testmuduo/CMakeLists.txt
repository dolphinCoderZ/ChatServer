cmake_minimum_required(VERSION 2.0)

project(muduo_server)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 设置可执行文件存储路径，PROJECT_SOURCE_DIR表示工程的根目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件搜索路径
# include_directories()
# 配置库文件搜索路径
# link_directories()
# 设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp)
# 把.指定路径下的所有源文件名字放入变量SRC_LIST
# aux_source_directory(. SRC_LIST)

# 生成可执行文件和需要的源文件
add_executable(server ${SRC_LIST})

# 目标文件需要链接的库文件
target_link_libraries(server muduo_net muduo_base pthread)