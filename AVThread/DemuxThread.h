#pragma once

#include "Common.h"

class DemuxThread final :public EventLoop
{
public:
	DemuxThread();
	virtual ~DemuxThread() {};

	int init(std::string const& filename,
		std::shared_ptr<PacketQueue> pVideo_packet_queue,
		std::shared_ptr<PacketQueue> pAudio_packet_queue);
	AVCodecParameters* getVideoCodecPara();
	AVCodecParameters* getAudioCodecPara();

	int seek(double percent);

protected:
	virtual int runOnce()override;
private:
	PacketPtr pPkt;
	std::shared_ptr<Demuxer> pDemuxer;
	std::shared_ptr<PacketQueue> pVideo_packet_queue_;
	std::shared_ptr<PacketQueue> pAudio_packet_queue_;

	uint32_t nVideo_index;
	uint32_t nAudio_index;
};

