extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}

#include "AVCore.h"

Converter::Converter(int src_pixel_format,int dst_pixel_format,int w,int h)
    :pswsContext(nullptr),
    width(w),
    height(h),
    dstPixelFormat(dst_pixel_format){
        std::cout << "src: " << src_pixel_format << "dst: " << dstPixelFormat <<std::endl;
        pswsContext = sws_getCachedContext(pswsContext,w,h,(AVPixelFormat)src_pixel_format,w,h,(AVPixelFormat)dst_pixel_format,SWS_BILINEAR,0,0,0);
        if(pswsContext == nullptr){
            std::cout << "sws_getCachedContext error" << std::endl;
        }
}

std::shared_ptr<Frame> Converter::Convert(std::shared_ptr<Frame> pFrame)
{
    std::string str;
    str.resize(128);
    av_get_pix_fmt_string(&(str[0]), 256, (AVPixelFormat)pFrame->getFormat());
    std::cout << "Pixel Format: " << str << std::endl;
    if (pFrame == nullptr) {
        std::cout << "pFrame is nullptr" << std::endl;
    }
    std::shared_ptr<Frame> pFrameYUV = std::make_shared<Frame>();

    int ret =
        av_image_alloc(pFrameYUV->frame->data, pFrameYUV->frame->linesize, width, height, (AVPixelFormat)dstPixelFormat, 1);

    if (ret < 0) {
        std::cout << "av_image_alloc error" << std::endl;
    }
    if (pswsContext == nullptr) {
        std::cout << "pswsContext is nullptr" << std::endl;
    }
    ret = sws_scale(pswsContext, pFrame->frame->data, pFrame->frame->linesize, 0, pFrame->frame->height, pFrameYUV->frame->data, pFrameYUV->frame->linesize);

    return pFrameYUV;
}

void Converter::Convert(std::shared_ptr<Frame> pFrame, AVFrame* FrameYUV)
{

    std::string str;
    str.resize(128);
    av_get_pix_fmt_string(&(str[0]), 256, (AVPixelFormat)pFrame->getFormat());
    std::cout << "Pixel Format:" << str << std::endl;
    if (pFrame == nullptr) {
        std::cout << "pFrame is nullptr" << std::endl;
    }
    int ret = sws_scale(pswsContext, pFrame->frame->data, pFrame->frame->linesize, 0, pFrame->frame->height, FrameYUV->data, FrameYUV->linesize);

}


