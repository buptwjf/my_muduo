cmake_minimum_required(VERSION 2.5)
project(mymuduo)

# cmake => make file => make
# mymuduo 最终编译成 so 动态库， 设置动态库的路径，放在根目录 lib 文件下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置调试信息，启动 C++11 标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

#定义参与编译的源代码文件 
aux_source_directory(. SRC_LIST)
#编译生成动态库mymuduo
add_library(mymuduo SHARED ${SRC_LIST} Poller.h DefaultPoller.cc Socket.h Channel.h)