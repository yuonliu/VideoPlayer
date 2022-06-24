extern "C"{
    #include <libavformat/avformat.h>
}

#include "AVCore.h"

Frame::Frame():frame(av_frame_alloc())
{}

Frame::~Frame()
{
    if(frame){
        av_frame_free(&frame);
        frame = nullptr;
    }
}