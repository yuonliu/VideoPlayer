#include "AVCore.h"
extern "C" {
#include <libavformat/avformat.h>
}

namespace AV {

Packet::Packet() { pkt = av_packet_alloc(); }
Packet::~Packet() {
  if (pkt) {
    av_packet_free(&pkt);
    pkt = nullptr;
  }
}
int Packet::GetIndex() { return pkt->stream_index; }
}  // namespace AV