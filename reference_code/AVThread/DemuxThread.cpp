#include "DemuxThread.h"

namespace AV {

DemuxThread::DemuxThread() {
  ThreadEventLoop::SetName("DemuxThread");
  pDemuxer_ = std::make_unique<Demuxer>();
  LOGIMPT("construct DemuxThread ");
}

DemuxThread::~DemuxThread() {
  ThreadEventLoop::BeforeDerivedClassDeconstruct();
  LOGIMPT("deconstruct DemuxThread ");
}

int DemuxThread::Init(std::string const& filename,
                      std::shared_ptr<PacketQueue> pVideo_packet_queue,
                      std::shared_ptr<PacketQueue> pAudio_packet_queue) {
  if (pVideo_packet_queue == nullptr || pAudio_packet_queue == nullptr) {
    LOGERROR("Thread %s Queue Ptr is nullptr", GetName().c_str());
  }
  int ret = pDemuxer_->Open(filename);
  LOGERROR_IF(ret, "Thread %s Open Error! filename = %s, ret = %d",
              GetName().c_str(), filename.c_str(), ret);
  nVideo_index_ = pDemuxer_->GetVideoStreamIndex();
  nAudio_index_ = pDemuxer_->GetAudioStreamIndex();
  LOGONTEST("Thread %s video stream index = %u, audio stream index = %u",
            GetName().c_str(), nVideo_index_, nAudio_index_);
  pVideo_packet_queue_ = pVideo_packet_queue;
  pAudio_packet_queue_ = pAudio_packet_queue;

  // std::weak_ptr<Demuxer> pWeakDemuxer(pDemuxer_);
  // std::function<void(double)> demuxer_seek_func = [pWeakDemuxer] (double
  // percent) {
  //   auto pSharedDemuxer = pWeakDemuxer.lock();
  //   if(pSharedDemuxer == nullptr) {
  //     LOGWARN("Demuxer has been deconstruct");
  //   }
  //   uint64_t seek_pts = pSharedDemuxer->Seek(percent);
  //   MessageBus::Notify("Decoder:seek_pts", seek_pts);
  // };
  // MessageBus::Attach(demuxer_seek_func, this, "");
  return 0;
}

AVCodecParameters* DemuxThread::GetVideoCodecPar() {
  return pDemuxer_->GetStreamCodecPar(pDemuxer_->GetVideoStreamIndex());
}

AVCodecParameters* DemuxThread::GetAudioCodecPar() {
  return pDemuxer_->GetStreamCodecPar(pDemuxer_->GetAudioStreamIndex());
}

int DemuxThread::Seek(double percent) {
  LOGERROR("Thread %s Seek is not implemented", GetName().c_str());
  return 0;
}

int DemuxThread::RunOnce() {
  LOGONTEST("Thread %s  RunOnce", GetName().c_str());

  // 如果有超时的数据需要push，则先push。push失败直接返回
  if (pPkt_ != nullptr) {
    if (pPkt_->GetIndex() == nVideo_index_) {
      if (pVideo_packet_queue_->push_timeout(pPkt_, 100) == false) {
        return 0;
      } else {
        pPkt_ = nullptr;
      }
    } else if (pPkt_->GetIndex() == nAudio_index_) {
      if (pAudio_packet_queue_->push_timeout(pPkt_, 100) == false) {
        return 0;
      } else {
        pPkt_ = nullptr;
      }
    }
  }

  pPkt_ = std::make_shared<Packet>();
  int ret = pDemuxer_->Read(pPkt_);
  if (ret == 1) {
    LOGIMPT("Thread %s no more packet", GetName().c_str());
    pVideo_packet_queue_->push(nullptr);
    pAudio_packet_queue_->push(nullptr);
    ThreadEventLoop::Finish();
  }
  LOGONTEST("Thread %s packet index = %d", GetName().c_str(),
            pPkt_->GetIndex());
  if (pPkt_->GetIndex() == nVideo_index_) {
    // LOGONTEST("Thread %s push video packet to queue", GetName().c_str());
    // LOGONTEST("Thread %s video packet queue, queue size = %u, capacity = %u",
    //           GetName().c_str(), pVideo_packet_queue_->size(),
    //           pVideo_packet_queue_->capacity());
    if (pVideo_packet_queue_->push_timeout(pPkt_, 100) == false) {
      LOGWARN("Thread %s video packet queue, queue size = %u, capacity = %u",
              GetName().c_str(), pAudio_packet_queue_->size(),
              pAudio_packet_queue_->capacity());
      LOGWARN("Thread %s try to push video packet queue timeout",
              GetName().c_str());
      return 0;
    } else {
      pPkt_ = nullptr;
    }
  } else if (pPkt_->GetIndex() == nAudio_index_) {
    // LOGONTEST("Thread %s push audio packet to queue", GetName().c_str());
    // LOGONTEST("Thread %s audio packet queue, queue size = %u, capacity = %u",
    //           GetName().c_str(), pAudio_packet_queue_->size(),
    //           pAudio_packet_queue_->capacity());
    if (pAudio_packet_queue_->push_timeout(pPkt_, 100) == false) {
      // LOGWARN("Thread %s audio packet queue, queue size = %u, capacity = %u",
      //         GetName().c_str(), pAudio_packet_queue_->size(),
      //         pAudio_packet_queue_->capacity());
      // LOGWARN("Thread %s try to push audio packet queue timeout",
      //         GetName().c_str());
      return 0;
    } else {
      pPkt_ = nullptr;
    }
  }
  return 0;
}

int DemuxThread::ClearImpl() {
  pPkt_ = nullptr;
  pDemuxer_->Clear();
  return 0;
}

}  // namespace AV