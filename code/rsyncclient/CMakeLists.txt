set(SRC_LIST main.cpp MainMod.cpp NetMod.cpp)

add_executable(rsyncclient ${SRC_LIST})

#添加链接库
target_link_libraries(rsyncclient LogModule NetUtil FileModule RsyncKernel TinyXml2 ThreadModule)