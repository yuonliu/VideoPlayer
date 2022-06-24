#include <iostream>

#include "AVCore.h"
extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}

namespace AV {
Frame::Frame() { frame = av_frame_alloc(); }

Frame::~Frame() {
  if (frame != nullptr) {
    av_frame_free(&frame);
    frame = nullptr;
  }
}

uint64_t Frame::GetPts() const {
  LOGERROR_IF(frame == nullptr, "frame is nullptr");
  return frame->pts;
}

int Frame::GetFormat() const {
  LOGERROR_IF(frame == nullptr, "frame is nullptr");
  return frame->format;
}
int Frame::VideoPrint() {
  LOGINFO("width %d\theight %d", frame->width, frame->height);
  AVPixelFormat format = (AVPixelFormat)(frame->format);
  std::string str;
  str.resize(256);
  av_get_pix_fmt_string(&(str[0]), 256, format);
  LOGONTEST("Pixel Format: %s", str.c_str());

  for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
    LOGONTEST("Linesize[%d]: %d", i, frame->linesize[i]);
  }
  return 0;
}

int Frame::AudioPrint() {
  int channel = frame->channels;
  LOGINFO("Channel: %d", channel);
  LOGINFO("nb_samples: %d", frame->nb_samples);
  LOGINFO("sample_rate: %d", frame->sample_rate);

  AVSampleFormat format = (AVSampleFormat)(frame->format);

  std::string str;
  str.resize(128);
  av_get_sample_fmt_string(&(str[0]), 128, format);
  LOGONTEST("Sample Format: %s", str.c_str());

  for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
    LOGONTEST("Linesize[%d]: %d", i, frame->linesize[i]);
  }
  return 0;
}

}  // namespace AV