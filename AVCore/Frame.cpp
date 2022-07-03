extern "C"{
    #include <libavformat/avformat.h>
    #include <libavutil/pixdesc.h>
}
#include <iostream>

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

uint64_t Frame::getPts() const
{
    if(frame == nullptr)
    {
        std::cout << "frame is nullptr" <<std::endl;
    }
    return frame->pts;
}

int Frame::getFormat() const
{
    if(frame == nullptr)
    {
        std::cout << "frame is nullptr" << std::endl;
    }
    return frame->format;
}


int Frame::videoPrint()
{
    std::cout << "width " << frame->width << " " << "height " << frame->height <<std::endl;
    AVPixelFormat format = (AVPixelFormat)(frame->format);
    std::string str;
    str.resize(256);
    av_get_pix_fmt_string(&(str[0]),256,format);
    std::cout << "Pixel Format: " << str.c_str() << std::endl;

    for(int i = 0; i < AV_NUM_DATA_POINTERS;i++)
    {
        std::cout << "linesize " << i <<frame->linesize[i] << std::endl;
    }
    return 0;
}

int Frame::audioPrint()
{
    int channel = frame->channels;
    std::cout << "Channel: " << channel << std::endl;
    std::cout << "nb_samples: " << frame->nb_samples << std::endl;
    std::cout << "sample rate:" << frame->sample_rate << std::endl;


    AVSampleFormat format = (AVSampleFormat)(frame->format);


    std::string str;
    str.resize(128);
    av_get_sample_fmt_string(&(str[0]),128,format);
    std::cout << "Sample Format: " << str.c_str() <<std::endl;

    for(int i = 0; i < AV_NUM_DATA_POINTERS;i++)
    {
        std::cout << "linesize " << i << " " << frame->linesize[i] << std::endl;
    }

    return 0;

}