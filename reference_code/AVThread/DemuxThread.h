#pragma once
#include "Common.h"

namespace AV {

class DemuxThread final : public ThreadEventLoop {
 public:
  DemuxThread();
  virtual ~DemuxThread();

  int Init(std::string const& filename,
           std::shared_ptr<PacketQueue> pVideo_packet_queue,
           std::shared_ptr<PacketQueue> pAudio_packet_queue);
  AVCodecParameters* GetVideoCodecPar();
  AVCodecParameters* GetAudioCodecPar();
  int Seek(double percent);

 protected:
  virtual int RunOnce() override;
  friend MediaPlayer;

 private:
  virtual int ClearImpl() override;

 private:
  PacketPtr pPkt_;
  std::shared_ptr<Demuxer> pDemuxer_;
  std::shared_ptr<PacketQueue> pVideo_packet_queue_;
  std::shared_ptr<PacketQueue> pAudio_packet_queue_;
  uint32_t nVideo_index_;
  uint32_t nAudio_index_;
};
}  // namespace AV