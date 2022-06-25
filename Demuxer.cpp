extern "C"{
    #include <libavformat/avformat.h>
}

#include "AVCore.h"

Demuxer::Demuxer():formatContext(avformat_alloc_context())
{
    std::cout << "construct Demuxer" << std::endl;
    
    avformat_network_init();

    if(!formatContext)
    {
        std::cout << "alloc format context failed" << std::endl;
    }
}

Demuxer::~Demuxer()
{

    std::cout << "deconstruct Demuxer" << std::endl;
    clear();
    if(formatContext)
    {
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
}

int Demuxer::open(std::string const& filename)
{
    if(!formatContext)
    {
        std::cout << "Open failed, because format context is nullptr" << std::endl;
    }
    if(formatContext == nullptr) return -1;
    int ret = avformat_open_input(&formatContext,filename.c_str(),nullptr,nullptr);
    if(ret)
    {
        std::cout << "avformat open input fail " << filename << std::endl;
    }else{
        avformat_find_stream_info(formatContext,nullptr);
        av_dump_format(formatContext,0,filename.c_str(),0);
    }

    infoMap.emplace("filename", filename);

    return ret;
}

double Demuxer::getInfo()
{
    if(formatContext == nullptr)
    {
        std::cout << "format context is nullptr" << std::endl;
    }
    infoMap.emplace("stream_number",std::to_string(formatContext -> nb_streams));

    int hours,mins,secs;
    secs = formatContext -> duration / 1000000;
    mins = secs / 60;
    secs %= 60;
    hours = mins /60;
    mins %= 60;
    char duration_format[128];

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
    int stream_index = -1;
    if(onlyAudio)
    {
        stream_index = getAudioStreamIndex();   
    }else {
        stream_index = getAudioStreamIndex();
    }
}



int Demuxer::getVideoStreamIndex()
{
    if(onlyAudio)
    {
        return -1;
    }

    return av_find_best_stream(formatContext,AVMediaType::AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
}

int Demuxer::getAudioStreamIndex()
{
    return av_find_best_stream(formatContext,AVMediaType::AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
}
