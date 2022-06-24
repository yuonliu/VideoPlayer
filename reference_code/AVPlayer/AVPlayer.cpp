#include "AVPlayer.h"

namespace AV {
MediaPlayer::MediaPlayer() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGIMPT("construct MediaPlayer begin");
  _Init();
  LOGIMPT("construct MediaPlayer end");
}

MediaPlayer::~MediaPlayer() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGIMPT("Begin deconstruct MediaPlayer");
  // oDemuxThread_.ExitLoop();
  // oAudioDecodeThread_.ExitLoop();
  // oVideoDecodeThread_.ExitLoop();
  // oVideoRenderThread_.ExitLoop();
  // oAudioRenderThread_.ExitLoop();
  LOGIMPT("deconstruct MediaPlayer");
}

void MediaPlayer::Open(std::string const& filename, void* windowHandle) {
  std::unique_lock<std::mutex> locker(mutex_);
  // 建立音视频同步对象
  pAVSynchronizer_ = std::make_shared<AudioVideoSynchronizer>();

  // 初始化线程
  LOGINFO("init mediaPlayer thread");
  int ret =
      oDemuxThread_.Init(filename, pVideoPacketQueue_, pAudioPacketQueue_);
  LOGERROR_IF(ret, "DemuxThread init error! with ret = %d", ret);
  oVideoDecodeThread_.SetName("VideoDecodeThread");

  // 判断是否存在视频
  auto videoCodecPar = oDemuxThread_.GetVideoCodecPar();
  onlyAudio_ = bool(videoCodecPar == nullptr);

  if (onlyAudio_ == false) {
    ret = oVideoDecodeThread_.Init(oDemuxThread_.GetVideoCodecPar(),
                                   pVideoPacketQueue_, pVideoFrameQueue_);
    LOGERROR_IF(ret, "VideoDecodeThread init error! with ret = %d", ret);
  }

  oAudioDecodeThread_.SetName("AudioDecodeThread");
  ret = oAudioDecodeThread_.Init(oDemuxThread_.GetAudioCodecPar(),
                                 pAudioPacketQueue_, pAudioFrameQueue_);
  LOGERROR_IF(ret, "AudioDecodeThread init error! with ret = %d", ret);

  if (onlyAudio_ == false) {
    ret = oVideoRenderThread_.Init(pVideoFrameQueue_, pAVSynchronizer_,
                                   windowHandle);
    LOGERROR_IF(ret, "VideoRenderThread init error! with ret = %d", ret);
  }

  ret = oAudioRenderThread_.Init(oDemuxThread_.GetAudioCodecPar(),
                                 pAudioFrameQueue_, pAVSynchronizer_);
  LOGERROR_IF(ret, "AudioRenderThread init error! with ret = %d", ret);
  oAudioRenderThread_.SetPCMQueue(pPCMQueue_);

  // 启动线程
  LOGINFO("Start mediaPlayer thread");
  oDemuxThread_.StartLoop();
  oAudioDecodeThread_.StartLoop();
  oVideoDecodeThread_.StartLoop();
  oVideoRenderThread_.StartLoop();
  oAudioRenderThread_.StartLoop();
}

void MediaPlayer::Play() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGINFO("Play action");
  oDemuxThread_.Resume();
  oAudioDecodeThread_.Resume();
  if (onlyAudio_ == false) {
    oVideoDecodeThread_.Resume();
  }
  // 等两秒让缓冲队列填满一点，这样播放更流畅
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  if (onlyAudio_ == false) {
    oVideoRenderThread_.Resume();
  }
  oAudioRenderThread_.Resume();
}

void MediaPlayer::Pause() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGINFO("Pause action");
  LOGIMPT("Begin Suspend");
  // oDemuxThread_.Suspend();
  // oAudioDecodeThread_.Suspend();
  // oVideoDecodeThread_.Suspend();

  oAudioRenderThread_.Suspend();
  if (onlyAudio_ == false) {
    oVideoRenderThread_.Suspend();
  }
}

void MediaPlayer::Close() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGINFO("Close action");
  oDemuxThread_.ExitLoop();
  oAudioDecodeThread_.ExitLoop();
  oAudioRenderThread_.ExitLoop();

  if (onlyAudio_ == false) {
    oVideoDecodeThread_.ExitLoop();
    oVideoRenderThread_.ExitLoop();
  }
}

void MediaPlayer::Seek(double percent) {
  std::unique_lock<std::mutex> locker(mutex_);
  // 如何保证原子性？加锁？
  // 1. 暂停demux线程，音频解码线程，视频解码线程，音频渲染线程，视频渲染线程
  LOGIMPT("before seek : Suspend");
  LOGIMPT("Call Demuxer suspend()");
  oDemuxThread_.Suspend();
  LOGIMPT("Call AudioDecode suspend()");
  oAudioDecodeThread_.Suspend();

  if (onlyAudio_ == false) {
    LOGIMPT("Call VideoDecode suspend()");
    oVideoDecodeThread_.Suspend();
    LOGIMPT("Call VideoRender suspend()");
    oVideoRenderThread_.Suspend();
  }

  LOGIMPT("Call AudioRender suspend()");
  oAudioRenderThread_.Suspend();

  // 2. 清空packet队列，frame队列
  LOGIMPT("before seek : clear queue buffer");
  pAudioPacketQueue_->clear();
  pVideoPacketQueue_->clear();
  pAudioFrameQueue_->clear();
  pVideoFrameQueue_->clear();

  // 3.
  // 清空解复用器读取缓冲，音频解码器缓存，视频解码器缓存，音频渲染缓存，视频渲染缓存
  LOGIMPT("before seek : clear thread buffer");
  oDemuxThread_.Clear();
  oAudioDecodeThread_.Clear();
  oAudioRenderThread_.Clear();
  if (onlyAudio_ == false) {
    oVideoDecodeThread_.Clear();
    oVideoRenderThread_.Clear();
  }

  // 4. 进行seek低精度seek。seek到最接近的上一个关键帧
  LOGIMPT("begin seek");
  // MessageBus::NotifyByTopic("Demuxer:seek", percent);
  uint64_t seek_pts = oDemuxThread_.pDemuxer_->Seek(percent);
  LOGIMPT("end seek");
  // MessageBus::NotifyByTopic("Decoder:set_seek_pts", seek_pts); //
  // 阻塞通知decoder要seek到某个位置 5.1.
  // 给解码器设置seek模式，参数是seek期望的pts
  // 运行到对应pts会自动清除seekmode标志位
  oAudioDecodeThread_.pDecoder_->SetSeekMode(seek_pts);

  if (onlyAudio_ == false) {
    oVideoDecodeThread_.pDecoder_->SetSeekMode(seek_pts);
  }

  // // 5.2. 刷新解码器上下文
  // oAudioDecodeThread_.pDecoder_->FlushBuffers(); // avcodec_flush_buffers
  // oVideoDecodeThread_.pDecoder_->FlushBuffers(); // avcodec_flush_buffers

  // 6. 恢复解复用线程，音频解码线程，视频解码线程 ()
  LOGIMPT("after seek : Resume");
  LOGIMPT("Begin Resume");
  oDemuxThread_.Resume();
  oAudioDecodeThread_.Resume();
  oAudioRenderThread_.Resume();

  if (onlyAudio_ == false) {
    oVideoDecodeThread_.Resume();
    oVideoRenderThread_.Resume();
  }

  LOGIMPT("Seek OK");
}

void MediaPlayer::SetSpeed(double speed_rate) {
  std::unique_lock<std::mutex> locker(mutex_);
  // 如何保证原子性？加锁？
  // 1. 暂停demux线程，音频解码线程，视频解码线程，音频渲染线程，视频渲染线程
  LOGIMPT("before seek : Suspend");
  LOGIMPT("Call Demuxer suspend()");
  oDemuxThread_.Suspend();
  LOGIMPT("Call AudioDecode suspend()");
  oAudioDecodeThread_.Suspend();
  if (onlyAudio_ == false) {
    LOGIMPT("Call VideoDecode suspend()");
    oVideoDecodeThread_.Suspend();
    LOGIMPT("Call VideoRender suspend()");
    oVideoRenderThread_.Suspend();
  }

  LOGIMPT("Call AudioRender suspend()");
  oAudioRenderThread_.Suspend();

  // 2. 清空packet队列，frame队列
  LOGIMPT("before seek : clear queue buffer");
  pAudioPacketQueue_->clear();
  pVideoPacketQueue_->clear();
  pAudioFrameQueue_->clear();
  pVideoFrameQueue_->clear();

  // 3.
  // 清空解复用器读取缓冲，音频解码器缓存，视频解码器缓存，音频渲染缓存，视频渲染缓存
  LOGIMPT("before seek : clear thread buffer");
  oDemuxThread_.Clear();
  oAudioDecodeThread_.Clear();
  oAudioRenderThread_.Clear();
  if (onlyAudio_ == false) {
    oVideoDecodeThread_.Clear();
    oVideoRenderThread_.Clear();
  }

  oAudioRenderThread_.pAudio_resample_->SetSpeedRate(speed_rate);

  // 6. 恢复解复用线程，音频解码线程，视频解码线程 ()
  LOGIMPT("after seek : Resume");
  LOGIMPT("Begin Resume");
  oDemuxThread_.Resume();
  oAudioDecodeThread_.Resume();
  oAudioRenderThread_.Resume();
  if (onlyAudio_ == false) {
    oVideoDecodeThread_.Resume();
    oVideoRenderThread_.Resume();
  }
}

void MediaPlayer::SetVolume(double volume) {
  oAudioRenderThread_.pAudio_render_->SetVolume(volume);
}

void MediaPlayer::SetPCMQueue(std::shared_ptr<PCMQueue> pPCMQueue) {
  std::unique_lock<std::mutex> locker(mutex_);
  pPCMQueue_ = pPCMQueue;
}

void MediaPlayer::NextFrame() {
  std::unique_lock<std::mutex> locker(mutex_);
  // 如果没有暂停需要先暂停
  if (onlyAudio_ == false) {
    LOGIMPT("Call VideoRender suspend()");
    oVideoRenderThread_.Suspend();
  }

  LOGIMPT("Call AudioRender suspend()");
  oAudioRenderThread_.Suspend();

  if (onlyAudio_ == false) {
    oVideoDecodeThread_.RunOnce();
  }

  oAudioDecodeThread_.RunOnce();
}

void MediaPlayer::ResetWindow(int _x,int _y,int _w,int _h){
  oVideoRenderThread_.pVideo_render_->ResetWindow(_x,_y,_w,_h);
}

VideoInfo MediaPlayer::GetVideoInfo() const {
  std::unique_lock<std::mutex> locker(mutex_);
  return oDemuxThread_.pDemuxer_->GetVideoInfo();
}

void MediaPlayer::_Init() {
  // 建立队列
  LOGINFO("build queue");
  pAudioPacketQueue_ = std::make_shared<PacketQueue>(25);
  pVideoPacketQueue_ = std::make_shared<PacketQueue>(15);
  pAudioFrameQueue_ = std::make_shared<FrameQueue>(10);
  pVideoFrameQueue_ = std::make_shared<FrameQueue>(5);
  // 队列信息
  LOGONTEST("AudioPacketQueue.capacity = %u", pAudioPacketQueue_->capacity());
  LOGONTEST("VideoPacketQueue.capacity = %u", pVideoPacketQueue_->capacity());
  LOGONTEST("AudioFrameQueue.capacity = %u", pAudioFrameQueue_->capacity());
  LOGONTEST("VideoFrameQueue.capacity = %u", pVideoFrameQueue_->capacity());
}

}  // namespace AV
