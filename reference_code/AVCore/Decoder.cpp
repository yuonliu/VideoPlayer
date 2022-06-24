#include <iostream>

#include "AVCore.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
namespace AV {
Decoder::Decoder() {
  LOGIMPT("construct decoder");
  codecCtx = avcodec_alloc_context3(nullptr);
  seekPts_ = 0;
  seeking = false;
}
Decoder::~Decoder() {
  LOGIMPT("deconstruct decoder");
  if (codecCtx) {
    avcodec_free_context(&codecCtx);
    codecCtx = nullptr;
  }
  seekPts_ = 0;
  seeking = false;
}

int Decoder::Init(AVCodecParameters* codepar) {
  avcodec_parameters_to_context(codecCtx, codepar);
  AVCodec const* avCodec = avcodec_find_decoder(codecCtx->codec_id);
  int ret = avcodec_open2(codecCtx, avCodec, nullptr);
  if (ret) {
    LOGERROR("打开解码器失败: ret = %d", ret);
    return ret;
  }
  seekPts_ = 0;
  seeking = false;
  return 0;
}

int Decoder::SendPacket(std::shared_ptr<Packet> pPkt) {
  if (pPkt == nullptr) return avcodec_send_packet(codecCtx, nullptr);
  return avcodec_send_packet(codecCtx, pPkt->pkt);
}
int Decoder::RecvFrame(std::shared_ptr<Frame> pFrame) {
  LOGERROR_IF(pFrame == nullptr, "pFrame is nullptr! ret = %d", -1);
  return avcodec_receive_frame(codecCtx, pFrame->frame);
}

int Decoder::SetSeekMode(uint64_t seek_pts) {
  seekPts_ = seek_pts;
  seeking = true;
  return 0;
}
void Decoder::ClearSeekMode() {
  seekPts_ = 0;
  seeking = false;
}
uint64_t Decoder::GetSeekPts() const { return seekPts_; }
bool Decoder::isSeekMode() const { return seeking; }

int Decoder::Close() {
  LOGINFO("close decoder");
  seekPts_ = 0;
  seeking = false;
  return avcodec_close(codecCtx);
}

int Decoder::Clear() {
  LOGERROR_IF(codecCtx == nullptr, "codec context is nullptr");
  LOGINFO("Clear avcodec buffer");
  avcodec_flush_buffers(codecCtx);
  return 0;
}
}  // namespace AV