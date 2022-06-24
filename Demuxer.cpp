extern "C"{
    #include <libavformat/avformat.h>
}

#include "AVCore.h"

Demuxer::Demuxer():formatContext(avformat_alloc_context())
{}

Demuxer::~Demuxer()
{
    if(formatContext)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
}

int Demuxer::open(std::string const& file_name)
{
    if(formatContext == nullptr) return -1;
    int ret = avformat_open_input(&formatContext,file_name.data(),nullptr,nullptr);
    return ret;
}

int Demuxer::reset()
{
    if(formatContext == nullptr) return -1;
    avformat_close_input(&formatContext);
    return 0;
}

int Demuxer::clear()
{
    if(formatContext == nullptr) return -1;
    avformat_flush(formatContext);
    return 0;
}

int Demuxer::read(std::shared_ptr<Packet> pPacket)
{
    if(formatContext == nullptr) return -1;
    int ret = av_read_frame(formatContext,pPacket->pkt);
    return ret;
}

uint64_t Demuxer::seek(double percent)
{
    avformat_flush(formatContext);
    int 
}
