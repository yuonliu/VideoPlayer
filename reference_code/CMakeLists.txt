# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("AVKernel")
set(CMAKE_PREFIX_PATH $ENV{QTDIR624_64}) # 环境变量 QTDIR624_64 = D:\Qt\6.2.4\msvc2019_64

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON) # Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Resource Compiler
set(CMAKE_AUTOUIC ON) # User Interface Compiler

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets MultiMedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets MultiMedia REQUIRED)
find_package(Qt6 COMPONENTS Multimedia MultimediaWidgets REQUIRED)

# 包含子项目。
add_subdirectory ("AVCore")
add_subdirectory ("AVThread")
add_subdirectory ("AVPlayer")
add_subdirectory("Test")
add_subdirectory("3rdParty/gflags") 