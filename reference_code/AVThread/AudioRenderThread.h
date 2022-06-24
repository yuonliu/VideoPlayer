#pragma once
#include <condition_variable>

#include "Common.h"

namespace AV {

class AudioRenderThread final : public ThreadEventLoop {
 public:
  friend MediaPlayer;
  AudioRenderThread();
  ~AudioRenderThread();

  int Init(AVCodecParameters* para,
           std::shared_ptr<FrameQueue> pInput_frame_queue,
           std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer);

  int SetPCMQueue(std::shared_ptr<PCMQueue> pPCMQueue);

 protected:
  virtual int RunOnce() override;

 private:
  virtual int SuspendImpl() override;
  virtual int ResumeImpl() override;
  virtual int ClearImpl() override;
  virtual int BeforeRun() override;

 private:
  std::shared_ptr<FrameQueue> pInput_frame_queue_;
  std::shared_ptr<PCMQueue> pPCMQueue_;
  std::unique_ptr<Resample> pAudio_resample_;
  std::unique_ptr<AudioRenderBase> pAudio_render_;
  std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer_;
};
}  // namespace AV