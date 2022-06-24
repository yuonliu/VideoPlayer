#include "AudioRenderThread.h"

namespace AV {

AudioRenderThread::AudioRenderThread() {
  LOGIMPT("construct AudioRenderThread");
  SetName("AudioRenderThread");
}
AudioRenderThread::~AudioRenderThread() {
  ThreadEventLoop::BeforeDerivedClassDeconstruct();
  pAudio_resample_->Close();
  pAudio_render_->Close();
  LOGIMPT("deconstruct AudioRenderThread");
}

int AudioRenderThread::Init(
    AVCodecParameters* para, std::shared_ptr<FrameQueue> pInput_frame_queue,
    std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer) {
  // 输入帧队列
  LOGERROR_IF(pInput_frame_queue == nullptr, "pInput_frame_queue_ is nullptr");
  pInput_frame_queue_ = pInput_frame_queue;

  // 音视频同步信息
  LOGERROR_IF(pAVSynchronizer == nullptr, "pAVSynchronizer is nullptr");
  pAVSynchronizer_ = pAVSynchronizer;

  int ret;

  // 音频重采样器
  pAudio_resample_ = std::make_unique<Resample>();
  LOGERROR_IF(para == nullptr, "audio codec parameters is nullptr");
  ret = pAudio_resample_->SetDefaultOutputFormat();
  LOGERROR_IF(ret, "set output format error, ret = %d", ret);
  ret = pAudio_resample_->Open(para);
  LOGERROR_IF(ret, "audio resample open error, ret = %d", ret);

  // 音频渲染器
  pAudio_render_ = std::make_unique<QtAudioRender>();
  pAudio_render_->sampleRate = pAudio_resample_->GetOriginSampleRate();
  pAudio_render_->channels = pAudio_resample_->channel_num;
  ret = pAudio_render_->Open();
  LOGERROR_IF(ret, "Audio_render open error, ret = %d", ret);

  return 0;
}

int AudioRenderThread::BeforeRun() { return 0; }

int AudioRenderThread::SetPCMQueue(std::shared_ptr<PCMQueue> pPCMQueue) {
  pAudio_render_->pPCMQue = pPCMQueue;
  return 0;
}

int AudioRenderThread::RunOnce() {
  // 取出音频帧
  LOGONTEST("AudioRenderThread");
  FramePtr pFrame;
  LOGONTEST("Thread %s frame queue, queue size = %u, capacity = %u",
            GetName().c_str(), pInput_frame_queue_->size(),
            pInput_frame_queue_->capacity());
  if (pInput_frame_queue_->pop_timeout(pFrame, 100) == false) {
    LOGWARN("Thread %s frame queue, queue size = %u, capacity = %u",
            GetName().c_str(), pInput_frame_queue_->size(),
            pInput_frame_queue_->capacity());
    LOGWARN("Thread %s try to pop frame queue timeout", GetName().c_str());
    return 0;
  };
  // while(pInput_frame_queue_->try_pop(pFrame) == false) {
  //   LOGONTEST("fail to get new frame, queue_size = %u",
  //   pInput_frame_queue_->size()); std::this_thread::yield();
  // };
  if (pFrame == nullptr) {
    LOGIMPT("Thread %s no more frame to render", GetName().c_str());
    ThreadEventLoop::Finish();
    return 0;
  }

  // 打印音频帧信息log
  LOGONTEST("print audio fram info");
  pFrame->AudioPrint();

  LOGONTEST("Set sync pts");
  // 设置音视频同步信息: 当前正在播放的音频pts
  pAVSynchronizer_->SetSyncPts(pFrame->GetPts() -
                               pAudio_render_->GetUnplayedDuration());
  // 通知"play progress" topic 当前正在播放的pts
  MessageBus::Notify("play progress", pAVSynchronizer_->GetSyncPts());

  // LOGIMPT("Audio pts = %llu, sync pts = %llu", pFrame->GetPts(),
  // pAVSynchronizer_->GetSyncPts());
  // if(pAVSynchronizer_->ShouldDiscardAudioFrame(pFrame->GetPts())) {
  //   LOGONTEST("Discard Audio frame");
  //   return 0;
  // }
  // uint32_t delay_duration =
  // pAVSynchronizer_->AudioFrameDelayDuration(pFrame->GetPts());
  // if(delay_duration != 0) {
  //   LOGONTEST("Delay Audio frame render: %ums", delay_duration);
  //   std::this_thread::sleep_for(std::chrono::milliseconds(delay_duration));
  // }

  // 重采样转换格式得到pcm数据
  // 通过改变采样率改变速度
  std::vector<uint8_t> pcm_data;
  LOGONTEST("Do Resample");
  pAudio_resample_->DoResample(pFrame, pcm_data);
  LOGONTEST("DoResample. Get PCM data, data size = %zu bytes", pcm_data.size());

  // 音频渲染器播放pcm数据，并且尝试送入音频波形图pcm数据队列
  while (true) {
    if (pAudio_render_->GetFreeBufferSize() >= pcm_data.size()) {
      int ret = pAudio_render_->Write(pcm_data);
      LOGONTEST("Render audio, ret = %d", ret);
      break;
    } else {
      // LOGTRACE("no buffer to place pcm data");
      std::this_thread::yield();
    }
  }

  return 0;
}

int AudioRenderThread::ClearImpl() {
  pAudio_render_->Clear();  // 清空渲染器中的缓冲数据。
  pAVSynchronizer_->Clear();
  if (pPCMQueue_ != nullptr) {
    pPCMQueue_->clear();
  }
  return 0;
}

int AudioRenderThread::SuspendImpl() {
  pAudio_render_->Suspend();
  pAVSynchronizer_->Suspend();
  return 0;
}

int AudioRenderThread::ResumeImpl() {
  pAudio_render_->Resume();
  pAVSynchronizer_->Resume();
  return 0;
}
}  // namespace AV