#include  <DemuxThread.h>

DemuxThread::DemuxThread()
{
	EventLoop::setName("DemuxThread");
	pDemuxer = std::make_unique<Demuxer>();

	std::cout << "construct DemuxThread" << std::endl;
}

DemuxThread::~DemuxThread()
{
	EventLoop::beforeDerivedClassDestruct();
	std::cout << "desturct DemuxThread" << std::endl;
}

int DemuxThread::init(std::string const& filename,
	std::shared_ptr<PacketQueue> pVideo_packet_queue,
	std::shared_ptr<PacketQueue> pAudio_packet_queue) {
	if (pVideo_packet_queue == nullptr || pAudio_packet_queue == nullptr) {
		std::cout << "Thread " << getName() << " Queue ptr is nullptr" << std::endl;
	}
	int ret = pDemuxer->open(filename);
	if (ret) {
		std::cout << "Thread: " << getName() << " open error" << std::endl;
	}

	nVideo_index = pDemuxer->getVideoStreamIndex();
	nAudio_index = pDemuxer->getAudioStreamIndex();

	std::cout << "video stream index = " << nVideo_index << std::endl;
	std::cout << "audio stream index = " << nAudio_index << std::endl;

	pVideo_packet_queue_ = pVideo_packet_queue;
	pAudio_packet_queue_ = pAudio_packet_queue;

	return 0;
}

AVCodecParameters* DemuxThread::getVideoCodecPara()
{
	return pDemuxer->getStreamCodecPar(pDemuxer->getVideoStreamIndex());
}

AVCodecParameters* DemuxThread::getAudioCodecPara()
{
	return pDemuxer->getStreamCodecPar(pDemuxer->getAudioStreamIndex());
}

int  DemuxThread::seek(double percent)
{
	std::cout << "seek do not implement" << std::endl;
	return 0;
}

int DemuxThread::runOnce()
{
	std::cout << "Thread: " << getName() << " runOnce" << std::endl;
	//do not understand clearly
	if (pPkt != nullptr) {
		if (pPkt->getIndex() == nVideo_index) {
			if (pVideo_packet_queue_->push_timeout(pPkt, 100) == false) {
				return 0;
			}
			else {
				pPkt = nullptr;
			}
		}
		else if (pPkt->getIndex() == nAudio_index) {
			if (pAudio_packet_queue_->push_timeout(pPkt, 100) == false) {
				return 0;
			}
			else {
				pPkt = nullptr;
			}
		}
	}

	pPkt = std::make_shared<Packet>();
	int ret = pDemuxer->read(pPkt);
	if (ret == 1) {
		std::cout << "Thread " << getName() << " no more packet" << std::endl;
		pVideo_packet_queue_->push(nullptr);
		pAudio_packet_queue_->push(nullptr);
		EventLoop::finish();
	}

	std::cout << "Thread " << getName() << " packet index = " << pPkt->getIndex() << std::endl;

	if (pPkt->getIndex() == nVideo_index) {
		if (pVideo_packet_queue_->push_timeout(pPkt, 100) == false) {
			return 0;
		}
		else {
			pPkt = nullptr;
		}
	}
	else if(pPkt->getIndex() == nAudio_index) {
		if (pAudio_packet_queue_->push_timeout(pPkt, 100) == false) {
			return 0;
		}
		else {
			pPkt = nullptr;
		}
	}
	return 0;
}

int DemuxThread::clearImpl()
{
	pPkt = nullptr;
	pDemuxer->clear();
	return 0;
}

