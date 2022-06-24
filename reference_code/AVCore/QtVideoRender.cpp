#include "AVCore.h"

// extern "C" {
//   #include <libavcodec/avcodec.h>
//   #include <libavcodec/avformat.h>
//   #include <libavcodec/swscale.h>
// }
void QtVideoRender::Clear() { LOGONTEST("QtVideoRender::Clear do nothing"); }
void QtVideoRender::Init(std::string const& title, uint32_t width,
                         uint32_t height,
                         void* windowHandle) {
  // pImgConvertContext_
  //   = sws_getContext(width, height, (AVPixelFormat)avpixelformat,
  //                    width, height, AV_PIX_FMT_RGB24,
  //                    SWS_BICUBIC, nullptr, nullptr, nullptr);
  // pDstFrame_->format = AV_PIX_FMT_RGB24;
  // pDstFrame_->width = width;
  // pDstFrame_->height = height;
  // int ret = av_frame_get_buffer(pDstFrame_, 0);
  // LOGERROR_IF(ret < 0, "fail to alloc dst frame buffer");
  // LOGONTEST("Init convert context");
  LOGONTEST("QtVideoRender::Init do nothing");
}
void QtVideoRender::Render(std::shared_ptr<AV::Frame> pFrame) {
  // FrameToQImage(pFrame);
  LOGONTEST("QtVideoRender::Render do nothing");
}

// QImage* QtVideoRender::FrameToQImage(std::shared_ptr<Frame> pFrame) {
//   // if(pImgConvertContext_ == nullptr) {
//   //   QtVideoRender::Init(pFrame->frame->width, pFrame->frame->width,
//   pFrame->frame->format);
//   // }
//   // sws_scale(pImgConvertContext_, (uint8_t
//   const*const*)(pFrame->frame->data),
//   //           pFrame->frame->linesize, 0, pFrame->frame->height,
//   pDstFrame_->data, pDstFrame_->linesize);
//   // QImage img(pDstFrame_->width, pDstFrame_->height,
//   QImage::Format_RGB888);
//   // for(int y = 0; y < pDstFrame_->height; ++y) {
//   //   memcpy(img.scanLine(y), pDstFrame_->data[0] +
//   y*(pDstFrame_->linesize[0]), pDstFrame_->linesize[0]);
//   // }
//   // LOGONTEST("Get QImage");
//   return img;
// }
