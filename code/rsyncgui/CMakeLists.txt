# Find the QtWidgets library
find_package(Qt5Widgets)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#同理，查找当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
#同理，查找当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")

#通过Ui文件生成对应的头文件，一定要添加
qt5_wrap_ui(WRAP_FILES ${UI_FILES})

#添加资源文件
#set(RCC_FILES rcc.qrc)

# Tell CMake to create the executable
add_executable(rsyncgui ${SRC_FILES} ${HEAD_FILES} ${WRAP_FILES})

# Use the Widgets module from Qt 5.
target_link_libraries(rsyncgui Qt5::Widgets)
