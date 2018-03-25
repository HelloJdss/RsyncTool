include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/3rd)

aux_source_directory(. dir)
add_library(NetUtil SHARED ${dir} ../cm_define.h ../cm_struct.h)