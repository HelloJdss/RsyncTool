include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/NetUtil)
include_directories(${PROJECT_SOURCE_DIR}/code/common/RsyncKernel)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd/flatbuffers)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd)
include_directories(${PROJECT_SOURCE_DIR}/code/common/)

add_executable(rsyncclient main.cpp MainMod.cpp MainMod.h
        ../common/cm_struct.h
        ../common/RsyncKernel/
        ../common/3rd/
        ../common/Protocol/Protocol_define.h)

#添加静态链接库
target_link_libraries(rsyncclient LogModule pthread NetUtil RsyncKernel)