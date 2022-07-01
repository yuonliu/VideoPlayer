cmake_minimum_required(VERSION 3.8)

project("VideoPlayer")



include_directories(./3rdParty/ffmpeg/include)
include_directories(./3rdParty/SDL/include)

link_directories(./3rdParty/ffmpeg/lib)
link_directories(./3rdParty/SDL/lib/x64)

add_executable(testDemo main.cpp Decoder.cpp Demuxer.cpp Frame.cpp Packet.cpp Resample.cpp AVSynchronizer.cpp "SDLVideoRender.cpp" "AVThread/EventLoop.cpp" "AVThread/EventLoop.h")



target_link_libraries(testDemo avformat avcodec avutil )



