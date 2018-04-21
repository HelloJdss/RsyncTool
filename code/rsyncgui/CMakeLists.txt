# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5Xml)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")

#通过Ui文件生成对应的头文件，一定要添加
qt5_wrap_ui(WRAP_FILES ${UI_FILES})

#添加资源文件
set(QRC_SOURCE_FILES res.qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

# Tell CMake to create the executable
add_executable(rsyncgui ${SRC_FILES} ${HEAD_FILES} ${WRAP_FILES} ${QRC_FILES} ${INCLUDE_DIR})

# Use the Widgets module from Qt 5.
target_link_libraries(rsyncgui Qt5::Widgets Qt5::Xml)
