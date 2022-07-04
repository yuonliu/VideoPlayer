extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}
#include "AVCore.h"


Decoder::Decoder()
{
    std::cout << "construct decoder" << std::endl;

    codecContext = avcodec_alloc_context3(nullptr);
    seekPts = 0;
    seeking = false;
}

Decoder::~Decoder()
{
    std::cout << "deconstruct decoder" << std::endl;
    if(codecContext){
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }

    seekPts = 0;
    seeking = false;
}

int Decoder::init(AVCodecParameters* codecpar)
{
    avcodec_parameters_to_context(codecContext,codecpar);
    AVCodec const* avCodec = avcodec_find_decoder(codecContext->codec_id);
    int ret = avcodec_open2(codecContext,avCodec,nullptr);
    if(ret){
        std::cout << "fail to initial codec ret = " << ret << std::endl;
        return ret;
    }

    seekPts = 0;
    seeking = false;
    return 0;
}

int Decoder::sendPacket(std::shared_ptr<Packet> pPkt)
{
    if(pPkt == nullptr) return avcodec_send_packet(codecContext,nullptr);
    return avcodec_send_packet(codecContext,pPkt->pkt);
}

int Decoder::recvFrame(std::shared_ptr<Frame> pFrame)
{
    if(pFrame == nullptr){
        std::cout << "pFrame is nullptr" << std::endl;
        return -1;
    }
    return avcodec_receive_frame(codecContext,pFrame->frame);
}

int Decoder::setSeekMode(uint64_t seek_pts)
{
    seekPts = seek_pts;
    seeking = true;
    return 0;
}

void Decoder::clearSeekMode()
{
    seekPts = 0;
    seeking = false;
}

uint64_t Decoder::getSeekPts() const{return seekPts;}

bool Decoder::isSeekMode() const{return seeking;}

int Decoder::close()
{
    std::cout << "close decoder" << std::endl;

    seekPts = 0;
    seeking = false;
    return avcodec_close(codecContext);
}

int Decoder::clear()
{
    if(codecContext == nullptr){
        std::cout << "codec context is nullptr" << std::endl;
    }

    std::cout << "Clear avcodec buffer" << std::endl;

    avcodec_flush_buffers(codecContext);
    return 0;
}
