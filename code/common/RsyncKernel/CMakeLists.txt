include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)
include_directories(${PROJECT_SOURCE_DIR}/code/common/Protocol)
include_directories(${PROJECT_SOURCE_DIR}/code/common/FileUtil)

aux_source_directory(. dir)
add_library(RsyncKernel SHARED ${dir})