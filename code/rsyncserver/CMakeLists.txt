include_directories(${PROJECT_SOURCE_DIR}/code/common/)
include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/NetUtil)

set(SRC_LIST main.cpp
        MainMod.cpp
        NetMod.cpp
        NetMod.h)

#添加可执行文件
add_executable(rsyncserver ${SRC_LIST} ../common/cm_define.h)

#添加静态链接库
target_link_libraries(rsyncserver LogModule pthread NetUtil)