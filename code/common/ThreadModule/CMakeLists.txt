include_directories(${PROJECT_SOURCE_DIR}/code/common/LogModule)

aux_source_directory(. dir)
add_library(ThreadModule SHARED ${dir})
target_link_libraries(ThreadModule LogModule pthread)