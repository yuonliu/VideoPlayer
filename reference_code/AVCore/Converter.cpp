extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include "AVCore.h"
namespace AV {

Converter::Converter(int src_pixel_format, int dst_pixel_format,int w, int h)
    : pSwsContext_(nullptr),
      Width_(w),
      Height_(h),
      dstPixelFormat_(dst_pixel_format) {
  LOGTRACE("from %d w=%d,h=%d to %d w=%d,h=%d", src_pixel_format, Width_, Height_,
           dst_pixel_format, Width_, Height_);
  pSwsContext_ = sws_getCachedContext(
      pSwsContext_, w, h, (AVPixelFormat)src_pixel_format, w, h,
      (AVPixelFormat)dst_pixel_format, SWS_BILINEAR, 0, 0, 0);
  LOGERROR_IF(pSwsContext_ == nullptr, "sws_getCachedContext error");
};

std::shared_ptr<AV::Frame> Converter::Convert(std::shared_ptr<AV::Frame> pFrame) {
  std::string str;
  str.resize(128);
  av_get_pix_fmt_string(&(str[0]), 256, (AVPixelFormat)pFrame->GetFormat());
  LOGIMPT("Pixel Format: %s", str.c_str());
  LOGERROR_IF(pFrame == nullptr, "pFrame is nullptr");
  std::shared_ptr<AV::Frame> pFrameYUV = std::make_shared<AV::Frame>();
  int ret =
      av_image_alloc(pFrameYUV->frame->data, pFrameYUV->frame->linesize,
                     Width_, Height_, (AVPixelFormat)dstPixelFormat_, 1);
  LOGERROR_IF(ret < 0, "av_image_alloc error");
  LOGERROR_IF(pSwsContext_ == nullptr, "pSwsContext_ is bullptr");
  ret = sws_scale(pSwsContext_, pFrame->frame->data, pFrame->frame->linesize, 0,
                  pFrame->frame->height, pFrameYUV->frame->data,
                  pFrameYUV->frame->linesize);
  return pFrameYUV;
}

void Converter::Convert(std::shared_ptr<AV::Frame> pFrame,AVFrame* FrameYUV)
{
  std::string str;
  str.resize(128);
  av_get_pix_fmt_string(&(str[0]), 256, (AVPixelFormat)pFrame->GetFormat());
  LOGIMPT("Pixel Format: %s", str.c_str());
  LOGERROR_IF(pFrame == nullptr, "pFrame is nullptr");

  int ret = sws_scale(pSwsContext_, pFrame->frame->data, pFrame->frame->linesize, 0,
                  pFrame->frame->height, FrameYUV->data, FrameYUV->linesize);
}

};  // namespace AV
