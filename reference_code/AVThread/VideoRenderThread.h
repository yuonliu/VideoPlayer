#pragma once
#include "Common.h"

namespace AV {

class VideoRenderThread final : public ThreadEventLoop {
 public:
  friend MediaPlayer;
  VideoRenderThread();
  ~VideoRenderThread();

  int Init(std::shared_ptr<FrameQueue> pInput_frame_queue,
           std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer,
           void* windowHandle);

 protected:
  virtual int RunOnce() override;

 private:
  virtual int ClearImpl() override;

 private:
  std::shared_ptr<FrameQueue> pInput_frame_queue_;
  std::unique_ptr<VideoRenderBase> pVideo_render_;
  std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer_;
};
}  // namespace AV
