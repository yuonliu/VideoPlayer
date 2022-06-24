# CMakeList.txt: MMAV 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_PREFIX_PATH $ENV{QTDIR624_64}) # 环境变量 QTDIR624_64 = D:\Qt\6.2.4\msvc2019_64
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Multimedia REQUIRED)
find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets REQUIRED)
set(CMAKE_AUTOMOC ON) # Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Resource Compiler
set(CMAKE_AUTOUIC ON) # User Interface Compiler
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 由于生成的C++文件都是放在编译目录里的（在Shadow Building中有别于源码目录），所以还需要将编译目录包含到Include目录

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 将源代码添加到此项目的可执行文件。
add_library(
    AVCore
    Demuxer.cpp
    Packet.cpp
    Decoder.cpp
    Frame.cpp
    Resample.cpp
    AudioVideoSynchronizer.cpp
    QtAudioRender.cpp
    QtVideoRender.cpp
    Converter.cpp
    SDLVideoRender.cpp
)

include_directories(../)
include_directories(../../)
include_directories(../3rdParty/ffmpeg/include)
include_directories(../3rdParty/SDL/include)
link_directories(../3rdParty/ffmpeg/lib)
link_directories(../3rdParty/SDL/lib/x64)
include_directories(D:/Qt/6.2.4/msvc2019_64/include/QtMultimedia)
target_link_libraries(AVCore PRIVATE Qt6::Widgets Qt6::MultimediaWidgets Qt6::Multimedia)
target_link_libraries(AVCore PRIVATE SDL2)
target_link_libraries(AVCore PRIVATE SDL2main)