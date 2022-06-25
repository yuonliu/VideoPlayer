#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <map>


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
    int VideoPrint();
    int AudioPrint();
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
    int sendPacket();
    int recvFrame();


    int clear();
    int close();

private:
    AVCodecContext* codecContext = nullptr;
    uint64_t seekPts = 0;
};


class AudioRender
{

};

class VideoRender
{

};