cmake_minimum_required(VERSION 3.8)

project("VideoPlayer")



include_directories(./3rdParty/include)
link_directories(./3rdParty/lib)

add_executable(testDemo main.cpp Decoder.cpp Demuxer.cpp Frame.cpp Packet.cpp)



target_link_libraries(testDemo avformat avcodec avutil)



