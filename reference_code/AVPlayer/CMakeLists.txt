cmake_minimum_required (VERSION 3.8)

# set (CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
# set (CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
include_directories(../)
include_directories(../../)

link_directories(../3rdParty/ffmpeg/lib)
link_directories(../3rdParty/SDL/lib/x64)

# add_executable (MediaPlayer "MediaPlayer.cpp" "MediaPlayer.h")

add_library(
  AVPlayer
  AVPlayer.cpp
)

target_link_libraries(AVPlayer AVThread AVCore)
target_link_libraries(AVPlayer avformat)
target_link_libraries(AVPlayer avcodec)
target_link_libraries(AVPlayer avutil)
target_link_libraries(AVPlayer avdevice)
target_link_libraries(AVPlayer swresample)
target_link_libraries(AVPlayer swscale)
target_link_libraries(AVPlayer SDL2main)
target_link_libraries(AVPlayer SDL2)