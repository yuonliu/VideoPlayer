#include "DecodeThread.h"

DecodeThread::DecodeThread() {
	pDecoder_ = std::make_unique<Decoder>();
	std::cout << "construct DecodeThread" << std::endl;
}

DecodeThread::~DecodeThread() 
{
	EventLoop::beforeDerivedClassDestruct();
	std::cout << "destruct DecodeThread" << std::endl;
}

int DecodeThread::init(AVCodecParameters* codecPar,
	std::shared_ptr<PacketQueue> pInput_packet_queue,
	std::shared_ptr<FrameQueue> pOutput_frame_queue){
	if (!codecPar) {
		std::cout << "CodecPar is nullptr" << std::endl;
	}
	if (!pInput_packet_queue) {
		std::cout << "pInput_packet_queue is nullptr" << std::endl;
	}
	if (!pOutput_frame_queue) {
		std::cout << "pOutput_frame_queue is nullptr" << std::endl;
	}

	int ret = pDecoder_->init(codecPar);
	if (ret) {
		std::cout << "Decoder init error, ret = " << ret << std::endl;
	}

	pInput_packet_queue_ = pInput_packet_queue;
	pOutput_frame_queue_ = pOutput_frame_queue;

	return 0;
}

int DecodeThread::runOnce()
{
	std::cout << "Thread " << getName() << " runOnce" << std::endl;

	if (pFrame_ != nullptr && pOutput_frame_queue_->push_timeout(pFrame_, 100) == false) {
		return 0;
	}

	pFrame_ = std::make_shared<Frame>();

	int ret = pDecoder_->recvFrame(pFrame_);

	if (ret != 0) {
		PacketPtr pPkt;
		if (pInput_packet_queue_->pop_timeout(pPkt, 100) == false) {
			return 0;
		}
		if (pPkt == nullptr) {
			EventLoop()::finish();
			return 0;
		}
		pDecoder_->sendPacket(pPkt);
		pDecoder_->recvFrame(pFrame_);
	}

	if (pDecoder_->isSeekMode()) {
		if (pFrame_->getPts() < pDecoder_->getSeekPts()) {
			return 0;
		}
		else {
			pDecoder_->clearSeekMode();
		}
	}

	if (pOutput_frame_queue_->push_timeout(pFrame_, 100) == false) {
		return 0;
	}
	else {
		pFrame = nullptr;
	}
	return 0;
}

int DecodeThread::clearImpl()
{
	pDecoder_->clear();
	return 0;
}