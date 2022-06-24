#include "DecodeThread.h"

namespace AV {

DecodeThread::DecodeThread() {
  pDecoder_ = std::make_unique<Decoder>();
  // ThreadEventLoop::SetName("DecodeThread");
  LOGIMPT("construct DecodeThread");
}
DecodeThread::~DecodeThread() {
  ThreadEventLoop::BeforeDerivedClassDeconstruct();
  LOGIMPT("deconstruct DecodeThread");
}

int DecodeThread::Init(AVCodecParameters* codecPar,
                       std::shared_ptr<PacketQueue> pInput_packet_queue,
                       std::shared_ptr<FrameQueue> pOutput_frame_queue) {
  LOGERROR_IF(!codecPar, "CodecPar is nullptr");
  LOGERROR_IF(!pInput_packet_queue, "pInput_packet_queue is nullptr");
  LOGERROR_IF(!pOutput_frame_queue, "pOutput_frame_queue is nullptr");

  int ret = pDecoder_->Init(codecPar);
  LOGERROR_IF(ret, "Decoder Init ERROR! ret = %d", ret);
  pInput_packet_queue_ = pInput_packet_queue;
  pOutput_frame_queue_ = pOutput_frame_queue;

  // std::weak_ptr<Decoder> pWeakDecoder(pDecoder_);
  // std::function<void(uint64_t)> decoder_seek_func = [pWeakDecoder] (uint64_t
  // seek_pts) {
  //   auto pSharedDecoder = pWeakDecoder.lock();
  //   if(pSharedDecoder == nullptr) {
  //     LOGWARN("Decoder has been deconstruct");
  //   }
  //   pSharedDecoder->SetSeekPts(seek_pts);
  //   pSharedDecoder->SkipUselessFrame();
  //   MessageBus::Notify("Decoder:seek_pts", seek_pts);
  // };
  // MessageBus::Attach(decoder_seek_func, this, "");
  return 0;
}

// TODO: 处理视频临近结尾部分的情况
int DecodeThread::RunOnce() {
  LOGONTEST("Thread %s Runonce", GetName().c_str());
  if (pFrame_ != nullptr &&
      pOutput_frame_queue_->push_timeout(pFrame_, 100) == false) {
    LOGWARN("Thread %s frame queue, queue size = %u, capacity = %u",
            GetName().c_str(), pOutput_frame_queue_->size(),
            pOutput_frame_queue_->capacity());
    LOGWARN("Thread %s try to push frame queue timeout", GetName().c_str());
    return 0;
  };
  pFrame_ = std::make_shared<Frame>();
  int ret = pDecoder_->RecvFrame(pFrame_);
  if (ret != 0) {
    LOGONTEST("Thread %s Send decoder new packet", GetName().c_str());
    PacketPtr pPkt;
    LOGONTEST("Thread %s packet queue, queue size = %u, capacity = %u",
              GetName().c_str(), pInput_packet_queue_->size(),
              pInput_packet_queue_->capacity());
    if (pInput_packet_queue_->pop_timeout(pPkt, 100) == false) {
      // LOGWARN("Thread %s packet queue, queue size = %u, capacity = %u",
      //         GetName().c_str(), pInput_packet_queue_->size(),
      //         pInput_packet_queue_->capacity());
      // LOGWARN("Thread %s try to pop packet queue timeout",
      // GetName().c_str());
      return 0;
    };
    if (pPkt == nullptr) {
      while (pOutput_frame_queue_->push_timeout(nullptr, 1000) == false) {
        LOGONTEST("Thread %s frame queue, queue size = %u, capacity = %u",
                  GetName().c_str(), pOutput_frame_queue_->size(),
                  pOutput_frame_queue_->capacity());
        LOGWARN("Thread %s try to push nullptr frame queue timeout",
                GetName().c_str());
      };
      LOGIMPT("Thread %s no more frame", GetName().c_str());
      ThreadEventLoop::Finish();
      return 0;
    }
    pDecoder_->SendPacket(pPkt);
    pDecoder_->RecvFrame(pFrame_);
    // TODO: 解码最后一个包
  }
  if (pDecoder_->isSeekMode()) {
    LOGONTEST("Thread %s Seeking", GetName().c_str());
    if (pFrame_->GetPts() < pDecoder_->GetSeekPts()) {
      LOGONTEST("Thread %s Skip this frame", GetName().c_str());
      LOGINFO("Thread %s this frame pts = %llu, seek pts = %llu",
              GetName().c_str(), pFrame_->GetPts(), pDecoder_->GetSeekPts());
      return 0;
    } else {
      LOGONTEST("Thread %s Clear Seek flag", GetName().c_str());
      pDecoder_->ClearSeekMode();
    }
  }
  // LOGONTEST("Thread %s frame queue, queue size = %u, capacity = %u",
  //           GetName().c_str(), pOutput_frame_queue_->size(),
  //           pOutput_frame_queue_->capacity());
  if (pOutput_frame_queue_->push_timeout(pFrame_, 100) == false) {
    // LOGWARN("Thread %s frame queue, queue size = %u, capacity = %u",
    //         GetName().c_str(), pOutput_frame_queue_->size(),
    //         pOutput_frame_queue_->capacity());
    // LOGWARN("Thread %s try to push frame queue timeout", GetName().c_str());
    return 0;
  } else {
    pFrame_ = nullptr;
  }
  return 0;
}

int DecodeThread::ClearImpl() {
  pDecoder_->Clear();  // 清空解码器中buffer数据。
  return 0;
}

}  // namespace AV