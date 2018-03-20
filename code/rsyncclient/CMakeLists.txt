include_directories(${PROJECT_SOURCE_DIR}/code/common/)
include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/NetUtil)

add_executable(rsyncclient main.cpp MainMod.cpp MainMod.h ../common/cm_struct.h)

#添加静态链接库
target_link_libraries(rsyncclient LogModule pthread NetUtil)