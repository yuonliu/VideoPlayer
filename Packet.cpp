extern "C"{
    #include <libavformat/avformat.h>
}

#include "AVCore.h"

Packet::Packet():pkt(av_packet_alloc())
{}

Packet::~Packet()
{
    if(pkt)
    {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
}

int Packet::getIndex()
{
    return pkt->stream_index;
}


