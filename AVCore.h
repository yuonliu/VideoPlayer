#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

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

class Demuxer
{
public:
    Demuxer();
    ~Demuxer();
    int open(std::string const&);
    int close();
    int reset();
    int clear();

    uint64_t seek(double percent);
    //uint64_t seek(time);

    int getStreamCount();

    AVCodecParameters* getStreamCodecPar();

    int getVideoStreamIndex();
    int getAudioStreamIndex();

    //someformat getVideoInfo() const;

    int read(std::shared_ptr<Packet>);

private:
    AVFormatContext* formatContext = nullptr;

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