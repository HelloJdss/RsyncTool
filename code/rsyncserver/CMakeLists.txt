#添加源
FILE(GLOB SRC_FILES "./*.cpp")
#添加头文件
FILE(GLOB HEAD_FILES "./*.h")

add_executable(rsyncserver ${SRC_FILES} ${HEAD_FILES})

#添加链接库
target_link_libraries(rsyncserver LogModule ThreadModule NetUtil FileModule RsyncKernel)