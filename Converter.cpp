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
        
}