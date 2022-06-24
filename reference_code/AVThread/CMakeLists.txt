cmake_minimum_required (VERSION 3.8)
set(CMAKE_PREFIX_PATH $ENV{QTDIR624_64}) # 环境变量 QTDIR624_64 = D:\Qt\6.2.4\msvc2019_64\lib\cmake
FIND_PACKAGE(Qt6 COMPONENTS Core Core5Compat Quick OpenGL Widgets Concurrent Svg Network QuickControls2 REQUIRED)
set(CMAKE_AUTOMOC ON) # Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Resource Compiler
set(CMAKE_AUTOUIC ON) # User Interface Compiler
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 由于生成的C++文件都是放在编译目录里的（在Shadow Building中有别于源码目录），所以还需要将编译目录包含到Include目录

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(../)
include_directories(../../)
add_library(
    AVThread
    ThreadEventLoop.cpp
    DemuxThread.cpp
    DecodeThread.cpp
    AudioRenderThread.cpp
    VideoRenderThread.cpp
    Common.h
)



target_link_libraries(AVThread AVCore)
target_link_libraries(AVThread Qt6::Widgets Qt6::Core Qt6::Core5Compat Qt6::Quick Qt6::OpenGL Qt6::Widgets Qt6::Concurrent Qt6::Svg Qt6::Network Qt6::QuickControls2)
