set(SRC_LIST main.cpp MainMod.cpp NetMod.cpp)

#添加可执行文件
add_executable(rsyncserver ${SRC_LIST})

#添加链接库
target_link_libraries(rsyncserver LogModule ThreadModule NetUtil FileModule RsyncKernel)