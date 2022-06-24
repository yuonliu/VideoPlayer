#pragma once
#include "Common.h"

namespace AV {

class DecodeThread final : public ThreadEventLoop {
 public:
  DecodeThread();
  ~DecodeThread();
  int Init(AVCodecParameters* codecPar,
           std::shared_ptr<PacketQueue> pInput_packet_queue,
           std::shared_ptr<FrameQueue> pOutput_frame_queue);

 protected:
  virtual int RunOnce() override;
  friend MediaPlayer;

 private:
  virtual int ClearImpl() override;

 protected:
  FramePtr pFrame_;  // 没有放入帧队列的帧
  std::shared_ptr<Decoder> pDecoder_;
  std::shared_ptr<PacketQueue> pInput_packet_queue_;
  std::shared_ptr<FrameQueue> pOutput_frame_queue_;
};
}  // namespace AV