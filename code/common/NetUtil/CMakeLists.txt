include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)

aux_source_directory(. dir)
add_library(NetUtil STATIC ${dir} ../LogModule MsgHelper.cpp MsgHelper.h ../cm_struct.h)