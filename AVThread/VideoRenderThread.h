#pragma once

#include "Common.h"

class VideoRenderThread final : public EventLoop
{
public:
	VideoRenderThread();
	~VideoRenderThread();

	int init(std::shared_ptr<FrameQueue> pInput_frame_queue,
		std::shared_ptr<AVSynchronizer> pAVSynchronizer,
		void* windowHandle);

protected:
	virtual int runOnce() override;

private:
	virtual int clearImpl() override;

private:
	std::shared_ptr<FrameQueue> pInput_frame_queue_;
	std::unique_ptr<VideoRenderBase> pVideo_render_;
	std::shared_ptr<AVSynchronizer> pAVSynchronizer_;
};