#include "VideoRenderThread.h"

namespace AV {

VideoRenderThread::VideoRenderThread() {
  LOGIMPT("construct VideoRenderThread");
  SetName("VideoRenderThread");
}
VideoRenderThread::~VideoRenderThread() {
  ThreadEventLoop::BeforeDerivedClassDeconstruct();
  LOGIMPT("deconstruct VideoRenderThread");
}

int VideoRenderThread::Init(
    std::shared_ptr<FrameQueue> pInput_frame_queue,
    std::shared_ptr<AudioVideoSynchronizer> pAVSynchronizer,
        void* windowHandle) {
  LOGERROR_IF(!pInput_frame_queue, "pInput_frame_queue is nullptr");
  pInput_frame_queue_ = pInput_frame_queue;
  LOGERROR_IF(pAVSynchronizer == nullptr, "pAVSynchronizer is nullptr");
  pAVSynchronizer_ = pAVSynchronizer;
  pVideo_render_ = std::make_unique<SDLVideoRender>();
  pVideo_render_->Init("mediaplayer test", 1920 / 2, 1080 / 2,windowHandle);
  return 0;
}

int VideoRenderThread::RunOnce() {
  LOGONTEST("VideoRenderThread");

  FramePtr pFrame;
  LOGONTEST("Thread %s frame queue, queue size = %u, capacity = %u",
            GetName().c_str(), pInput_frame_queue_->size(),
            pInput_frame_queue_->capacity());
  if (pInput_frame_queue_->pop_timeout(pFrame, 100) == false) {
    LOGWARN("Thread %s frame queue, queue size = %u, capacity = %u",
            GetName().c_str(), pInput_frame_queue_->size(),
            pInput_frame_queue_->capacity());
    LOGWARN("Thread %s try to pop video frame queue timeout",
            GetName().c_str());
    return 0;
  }
  if (pFrame == nullptr) {
    // 视频到末尾了
    LOGIMPT("Thread %s no more frame to render", GetName().c_str());
    ThreadEventLoop::Finish();
    return 0;
  }

  // 打印视频帧信息log
  pFrame->VideoPrint();

  // 音画同步

  if (pAVSynchronizer_->ShouldDiscardVideoFrame(pFrame->GetPts())) {
    LOGONTEST("discard video frame");
    LOGTRACE("ShouldDiscardVideoFrame Video pts = %llu, sync pts = %llu",
             pFrame->GetPts(), pAVSynchronizer_->GetSyncPts());
    return 0;
  }
  uint32_t delay_duration =
      pAVSynchronizer_->VideoFrameDelayDuration(pFrame->GetPts());
  if (delay_duration != 0) {
    LOGTRACE("Delay video frame render: %ums", delay_duration);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_duration));
  }
  // 渲染视频帧
  pVideo_render_->Render(pFrame);
  LOGTRACE("video frame pts = %llu", pFrame->GetPts());

  return 0;
}

int VideoRenderThread::ClearImpl() {
  pVideo_render_->Clear();  // 清空渲染器中的缓冲数据。
  return 0;
}
}  // namespace AV
