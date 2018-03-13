aux_source_directory(. dir)
add_library(LogModule STATIC ${dir} LogHelper.cpp LogHelper.h)