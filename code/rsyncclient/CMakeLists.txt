#添加源
FILE(GLOB SRC_FILES "./*.cpp")
#添加头文件
FILE(GLOB HEAD_FILES "./*.h")

add_executable(rsyncclient ${SRC_FILES} ${HEAD_FILES})

#添加链接库
target_link_libraries(rsyncclient LogModule NetUtil FileModule RsyncKernel TinyXml2 ThreadModule)