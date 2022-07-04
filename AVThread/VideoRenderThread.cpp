#include "VideoRenderThread.h"

VideoRenderThread::VideoRenderThread()
{
	std::cout << "construct VideoRenderThread" << std::endl;
	setName("VideoRenderThread");
}

VideoRenderThread::~VideoRenderThread()
{
	EventLoop::beforeDerivedClassDestruct();
	std::cout << "deconstruct VideoRenderThread" << std::endl;
}

int VideoRenderThread::init(
	std::shared_ptr<FrameQueue> pInput_frame_queue,
	std::shared_ptr<AVSynchronizer> pAVSynchronizer,
	void* windowHandle) {
	pInput_frame_queue_ = pInput_frame_queue;
	pAVSynchronizer_ = pAVSynchronizer;
	pVideo_render_ = std::make_unique<SDLVideoRender>();
	pVideo_render_->init("mediaplayer test", 1920 / 2, 1080 / 2, windowHandle);
	return 0;
}

int VideoRenderThread::runOnce()
{
	FramePtr pFrame;
	if (pInput_frame_queue_->pop_timeout(pFrame, 100) == false) {
		return 0;
	}
	if (pFrame == nullptr) {
		EventLoop::finish();
		return 0;
	}

	pFrame->videoPrint();

	if (pAVSynchronizer_->shouldDiscardVideoFrame(pFrame->getPts())) {
		return 0;
	}
	uint32_t delay_duration = pAVSynchronizer_->VideoFrameDealyDuration(pFrame->getPts());
	if (delay_duration != 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_duration));
	}

	pVideo_render_->render(pFrame);

	return 0;
}

int VideoRenderThread::clearImpl()
{
	pVideo_render_->clear();
	return 0;
}