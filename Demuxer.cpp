extern "C"{
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
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

    getInfo();
    for(auto const&t:infoMap){
        std::cout << "key = " << t.first.c_str()<<"\t\t" << "value = " << t.second.c_str() << std::endl;
    }

    totalDuration = 
        static_cast<uint32_t>(formatContext->duration/(AV_TIME_BASE/1000));
        std::cout << "total duration = " << totalDuration << std::endl;
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
    sprintf(duration_format, "%d:%d:%d",hours,mins,secs);
    infoMap.emplace("duration",duration_format);

    infoMap.emplace("bit rate",std::to_string(formatContext->bit_rate));

    AVDictionaryEntry* iter = nullptr;

    while((
        iter = av_dict_get(formatContext->metadata,"",iter,AV_DICT_IGNORE_SUFFIX)
    )){
        infoMap.emplace(iter->key, iter->value);
    }

    for(uint32_t i = 0; i < formatContext->nb_streams;++i)
    {
        AVStream* input_stream = formatContext->streams[i];
        if(input_stream -> codecpar ->codec_type == AVMEDIA_TYPE_VIDEO){
            
            if(input_stream->avg_frame_rate.den == 0)
            {
                onlyAudio = true;
                continue;
            }
            infoMap.emplace("frame rate",std::to_string(input_stream->avg_frame_rate.num/input_stream->avg_frame_rate.den));

            AVCodecParameters* codec_par = input_stream->codecpar;
            
            infoMap.emplace("width",std::to_string(codec_par->width));
            infoMap.emplace("height",std::to_string(codec_par->height));
            infoMap.emplace("video average bit rate",std::to_string(codec_par->bit_rate));

            AVCodecContext * avctx_video;
            //why the parameter can be NULL ?
            avctx_video = avcodec_alloc_context3(nullptr);
            int ret = avcodec_parameters_to_context(avctx_video,codec_par);
            if(ret < 0)
            {
                avcodec_free_context(&avctx_video);
                return 0;
            }
            char buf[128];
            avcodec_string(buf,sizeof(buf),avctx_video,0);
            infoMap.emplace("video_format",avcodec_get_name(codec_par->codec_id));
        }else if(input_stream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){
            AVCodecParameters* codec_par = input_stream -> codecpar;
            AVCodecContext* avctx_audio;
            avctx_audio = avcodec_alloc_context3(nullptr);
            int ret = avcodec_parameters_to_context(avctx_audio,codec_par);
            if(ret < 0){
                avcodec_free_context(&avctx_audio);
                return 0;
            }

            infoMap.emplace("avdio format",avcodec_get_name(avctx_audio->codec_id));
            infoMap.emplace("audio average bit rate",
                            std::to_string(codec_par->bit_rate));
            infoMap.emplace("channel nums", std::to_string(codec_par->channels));
            infoMap.emplace("Sample rate", std::to_string(codec_par->sample_rate));
        }
    }
    return 0;
}


int Demuxer::close()
{
    std::cout << "close demuxer" << std::endl;
    return reset();
}


int Demuxer::reset()
{
    if(formatContext == nullptr) return -1;
    avformat_close_input(&formatContext);
    totalDuration = 0;
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
    if(!formatContext){
        std::cout << "format context is nullptr" << std::endl;
    }
    int ret = av_read_frame(formatContext,pPacket->pkt);
    if(ret == AVERROR_EOF) return 1;
    if(ret){
        std::cout << "read frame error. ret = " << ret << std::endl;
    }

    pPacket->pkt->pts = static_cast<int64_t>(
        pPacket->pkt->pts*(1000*av_q2d(formatContext->streams[pPacket->pkt->stream_index]->time_base))
    );
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
    int64_t seek_pos = static_cast<int64_t>
        (formatContext->streams[stream_index]->duration * percent);
    //can't understand the symbol '|'
    av_seek_frame(formatContext, stream_index, seek_pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    return static_cast<uint64_t>(percent * totalDuration);
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


int Demuxer::getStreamCount() { return formatContext->nb_streams; }

AVCodecParameters* Demuxer::getStreamCodecPar(uint32_t index) {
    if (index >= formatContext->nb_streams) return nullptr;
    AVCodecParameters* codecpar = avcodec_parameters_alloc();
    avcodec_parameters_copy(codecpar, formatContext->streams[index]->codecpar);
    return codecpar;
}


 VideoInfo Demuxer::getVideoInfo() const {
    return infoMap;
}
