cmake_minimum_required (VERSION 3.8)

set(CMAKE_PREFIX_PATH $ENV{QTDIR624_64}) # 环境变量 QTDIR624_64 = D:\Qt\6.2.4\msvc2019_64

include_directories(../)
include_directories(../../)
include_directories(../3rdParty/SDL/include)
link_directories(../3rdParty/ffmpeg/lib)
link_directories(../3rdParty/SDL/lib/x64)
SET(CMAKE_BUILD_TYPE "Debug") # debug模式 or Release

# FIND_PACKAGE(Qt6 COMPONENTS Core Core5Compat Quick OpenGL Widgets Concurrent Svg Network QuickControls2 REQUIRED)
set(CMAKE_AUTOMOC ON) # Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Resource Compiler
set(CMAKE_AUTOUIC ON) # User Interface Compiler
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 由于生成的C++文件都是放在编译目录里的（在Shadow Building中有别于源码目录），所以还需要将编译目录包含到Include目录

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_executable (MediaPlayerTest "main.cpp")
qt_add_executable(TestMediaPlayer
  MANUAL_FINALIZATION
  "TestAVPlayer.cpp"
)

qt_add_executable(TestAVSync
  MANUAL_FINALIZATION
  "TestAVSync.cpp"
)

qt_add_executable(TestSDL
  MANUAL_FINALIZATION
  "TestSDL.cpp"
)

qt_finalize_executable(TestMediaPlayer)
qt_finalize_executable(TestAVSync)

target_link_libraries(TestMediaPlayer PRIVATE AVPlayer)
target_link_libraries(TestMediaPlayer PRIVATE gflags::gflags)

target_link_libraries(TestAVSync PRIVATE AVPlayer)

target_link_libraries(TestSDL PRIVATE SDL2)
target_link_libraries(TestSDL PRIVATE SDL2main)
# target_link_libraries(TestMediaPlayer avformat)
# target_link_libraries(TestMediaPlayer avcodec)
# target_link_libraries(TestMediaPlayer avutil)
# target_link_libraries(TestMediaPlayer avdevice)
# target_link_libraries(TestMediaPlayer swresample)
# target_link_libraries(TestMediaPlayer swscale)

