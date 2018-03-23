include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)

aux_source_directory(. dir)
add_library(RsyncKernel STATIC ${dir} RollingChecksum.cpp RollingChecksum.h Generator.cpp Generator.h Inspector.cpp Inspector.h)