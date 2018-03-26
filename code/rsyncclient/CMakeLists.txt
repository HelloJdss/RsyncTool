include_directories(${PROJECT_SOURCE_DIR}/code/common/FileUtil)
include_directories(${PROJECT_SOURCE_DIR}/code/common/Protocol)
include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/NetUtil)
include_directories(${PROJECT_SOURCE_DIR}/code/common/RsyncKernel)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd/flatbuffers)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd)
include_directories(${PROJECT_SOURCE_DIR}/code/common/)

set(SRC_LIST main.cpp MainMod.cpp NetMod.cpp)

add_executable(rsyncclient ${SRC_LIST})

#添加链接库
target_link_libraries(rsyncclient LogModule pthread NetUtil RsyncKernel FileModule)