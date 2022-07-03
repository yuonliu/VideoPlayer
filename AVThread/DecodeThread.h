#pragma once
#include "Common.h"

class DecodeThread final :public EventLoop
{
public:
	DecodeThread();
	~DecodeThread() {};
	int init(AVCodecParameters* codecPar,
		std::shared_ptr<PacketQueue> pInput_packet_queue,
		std::shared_ptr<FrameQueue> pOutput_frame_queue);
protected:
	virtual int runOnce() override;

private:
	virtual int clearImpl() override;

protected:
	FramePtr pFrame;
	std::shared_ptr<Decoder> pDecoder_;
	std::shared_ptr<PacketQueue> pInput_packet_queue_;
	std::shared_ptr<FrameQueue> pOutput_frame_queue_;

};