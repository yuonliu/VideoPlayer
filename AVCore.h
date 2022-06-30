#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <map>
#include <mutex>

#include <queue>


struct AVPacket;
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
struct AVRational;
struct SwsContext;

class Packet
{
public:
    Packet();
    ~Packet();
    int getIndex();
public:
    AVPacket* pkt = nullptr;

};

using VideoInfo = std::map<std::string, std::string>;

class Demuxer
{
public:
    Demuxer();
    ~Demuxer();
    int open(std::string const& filename);
    int close();
    int reset();
    int clear();

    uint64_t seek(double percent);
    //uint64_t seek(time);

    int getStreamCount();

    AVCodecParameters* getStreamCodecPar(uint32_t index);

    int getVideoStreamIndex();
    int getAudioStreamIndex();

    VideoInfo getVideoInfo() const;

    int read(std::shared_ptr<Packet>);

private:
    double getInfo();

private:
    AVFormatContext* formatContext = nullptr;
    bool onlyAudio = false;
    uint32_t totalDuration = 0;
    VideoInfo infoMap;
};

class Frame
{
public:
    Frame();
    ~Frame();
    int videoPrint();
    int audioPrint();
    uint64_t getPts() const;
    int getFormat() const;

public:
    AVFrame* frame = nullptr;    
};


class Decoder
{
public:
    Decoder();
    ~Decoder();

    int init(AVCodecParameters*);
    int sendPacket(std::shared_ptr<Packet> pPkt);
    int recvFrame(std::shared_ptr<Frame> pFrame);

    int setSeekMode(uint64_t seek_pts);
    void clearSeekMode();
    uint64_t getSeekPts() const;
    bool isSeekMode()const;

    int clear();
    int close();

private:
    AVCodecContext* codecContext = nullptr;
    uint64_t seekPts = 0;
    bool seeking = false;
};

class Resample
{
public:
    int sample_rate = 0;
    int nb_samples = 0;
    int channel_num = 2;
    Resample();
    ~Resample();
    int open(AVCodecParameters* para);
    int close();
    int doResample(std::shared_ptr<Frame>, std::vector<uint8_t>& data);
    int setOutputFormat(uint32_t format);
    int setDefaultOutputFormat();
    int setSpeedRate(double speed_rate);
    int getOriginalSampleRate();
    int clear();
    
private:
    uint32_t outputFormat;
    SwrContext* pswrContext;
    AVCodecParameters* codecPara;
    mutable std::mutex mtx;

};


class Converter
{
public:
    Converter(int src_pixel_format, int dst_pixel_format, int w, int h);
    //I don't know how to understand the "= default"
    ~Converter() = default;

    std::shared_ptr<Frame> Convert(std::shared_ptr<Frame> pFrame);
    void Convert(std::shared_ptr<Frame> pFrame, AVFrame* FrameYUV);


private:
    SwsContext* pswsContext;
    int width;
    int height;
    int dstPixelFormat;
};

class AVSynchronizer
{
    AVSynchronizer();
    ~AVSynchronizer() = default;

    uint64_t getSyncPts();
    void setSyncPts(uint64_t syncpts);
    void suspend();
    void resume();
    void clear();
    void setSpeedRate(double rate = 1.0);
    bool shouldDiscardVideoFrame(uint64_t video_frame_pts);
    uint32_t VideoFrameDelayDuration(uint64_t video_frame_pts);
    bool shouldDiscardAudioFrame(uint64_t audio_frame_pts);
    uint32_t AudioFrameDelayDuration(uint64_t audio_frame_pts);

private:
    uint64_t syncPts;
    mutable std::mutex mtx;
};



class AudioRenderBase
{
public:
    int sampleRate = 44100;//why is 44100?
    int sampleSize = 16;
    int channels = 2;
    double volume = 1.0;

    //before to understand the concurrentqueue use queue instead
    std::shared_ptr<std::queue<std::vector<uint8_t>>> pPCMQue;

    AudioRenderBase() = default;
    virtual ~AudioRenderBase(){}
    virtual int open() = 0;
    virtual int close() = 0;
    virtual int clear() = 0;
    virtual uint64_t getUnplayedDuration() = 0;
    virtual int write(std::vector<uint8_t> const& pcm_data) = 0;
    void setVolume(double vol) {
        volume = vol;
    }
    virtual int suspend() = 0;
    virtual int resume() = 0;
    virtual int getFreeBufferSize() = 0;
};

class VideoRenderBase
{
public:
    VideoRenderBase() = default;
    ~VideoRenderBase(){}
    virtual void clear() = 0;
    virtual void init(std::string const& title, uint32_t width, uint32_t height, void* windowHandle) = 0;

    virtual void render(std::shared_ptr<Frame> pFrame) = 0;
    virtual void resetWindow(int x, int y, int w, int h) = 0;
};

struct SDL_Window;
struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

class SDLVideoRender final :public VideoRenderBase
{
public:
    SDLVideoRender();
    virtual ~SDLVideoRender();
    virtual void clear() override;

};

