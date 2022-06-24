/**
 * @file AVCore.h
 * @author yilongdong (dyl20001223@163.com)
 * @date 2022-04-27
 * @brief 播放器内核核心类，相对稳定
 * @details 包含Packet Frame Demuxer Decoder Resample AudioRenderBase
 * VideoRenderBase QtAudioRender QtVideoRender AudioVideoSynchronizer
 * @version 0.1
 * @copyright Copyright (c) 2022
 * TODO: 1. 渲染器
 */
#pragma once
#include <chrono>
#include <cinttypes>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Common/ConcurrentQueue.h"
#include "Log/Log.h"

#define QT_NO_DEBUG_OUTPUT
#define QT_NO_INFO_OUTPUT
#define QT_NO_WARNING_OUTPUT

struct AVPacket;
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVCodecParameters;
struct AVFrame;
struct SwrContext;
struct AVRational;
struct SwsContext;

namespace AV {

class Packet {
 public:
  Packet();
  ~Packet();
  int GetIndex();

 public:
  AVPacket* pkt = nullptr;
};

using VideoInfo = std::map<std::string, std::string>;

class Demuxer {
 public:
  Demuxer();
  ~Demuxer();
  int Open(std::string const& filename);
  int Close();
  int Reset();
  int Clear();

  // 0.0 ~ 1.0
  uint64_t Seek(double percent);

  int GetStreamCount();

  AVCodecParameters* GetStreamCodecPar(uint32_t index);

  int GetVideoStreamIndex();
  int GetAudioStreamIndex();
  std::map<std::string, std::string> GetVideoInfo() const;

  // 到达文件末尾时返回1
  int Read(std::shared_ptr<Packet> pPacket);

 private:
  double _GetInfo();

 private:
  AVFormatContext* formatCtx = nullptr;
  uint32_t totalDuration_ = 0;
  bool onlyAudio_ = false;
  VideoInfo infoMap_;
};

class Frame {
 public:
  Frame();
  ~Frame();
  int VideoPrint();
  int AudioPrint();
  uint64_t GetPts() const;
  int GetFormat() const;

 public:
  AVFrame* frame = nullptr;
};

class Decoder {
 public:
  Decoder();
  ~Decoder();

  int Init(AVCodecParameters*);
  int SendPacket(std::shared_ptr<Packet> pPkt);
  int RecvFrame(std::shared_ptr<Frame> pFrame);
  int SetSeekMode(uint64_t seek_pts);
  void ClearSeekMode();
  uint64_t GetSeekPts() const;
  bool isSeekMode() const;
  int Clear();
  int Close();

 private:
  AVCodecContext* codecCtx = nullptr;
  uint64_t seekPts_ = 0;
  bool seeking = false;
};

class Resample {
 public:
  int sample_rate = 0;
  int nb_samples = 0;
  int channel_num = 2;
  Resample();
  ~Resample();
  int Open(AVCodecParameters* para);
  int Close();
  int DoResample(std::shared_ptr<Frame>, std::vector<uint8_t>& data);
  int SetOutputFormat(uint32_t format);
  int SetDefaultOutputFormat();
  int SetSpeedRate(double speed_rate);
  int GetOriginSampleRate();
  int Clear();

 private:
  uint32_t outputFormat_;  // int -> AVSampleFormat
  SwrContext* swrContext_;
  AVCodecParameters* codecPara_;
  mutable std::mutex mutex_;
};

class Converter {
 public:
  Converter(int src_pixel_format, int dst_pixel_format, int w, int h);
  ~Converter() = default;

  std::shared_ptr<AV::Frame> Convert(std::shared_ptr<AV::Frame> pFrame);
  void Convert(std::shared_ptr<AV::Frame> pFrame,AVFrame* FrameYUV);

 private:
  SwsContext* pSwsContext_;
  int Width_;
  int Height_;
  int dstPixelFormat_;
};

class AudioVideoSynchronizer {
 public:
  AudioVideoSynchronizer();
  ~AudioVideoSynchronizer() = default;
  uint64_t GetSyncPts();
  void SetSyncPts(uint64_t syncpts);
  void Suspend();
  void Resume();
  void Clear();
  void SetSpeedRate(double rate = 1.0);                    // 速度系数
  bool ShouldDiscardVideoFrame(uint64_t video_frame_pts);  // 是否应该丢弃视频帧
  uint32_t VideoFrameDelayDuration(
      uint64_t video_frame_pts);  // 是否应该延迟视频帧, 返回延迟时间，单位毫秒
  bool ShouldDiscardAudioFrame(uint64_t audio_frame_pts);  // 是否应该丢弃音频帧
  uint32_t AudioFrameDelayDuration(
      uint64_t audio_frame_pts);  // 是否应该延迟音频帧, 返回延迟时间，单位毫秒

 private:
  uint64_t sync_pts_;
  mutable std::mutex mutex_;
};

class AudioRenderBase {
 public:
  int sampleRate = 44100;  // 采样率
  int sampleSize = 16;     // 样本大小
  int channels = 2;        // 通道数
  double volume = 1.0;
  std::shared_ptr<ConcurrentQueue<std::vector<uint8_t>>> pPCMQue;

  AudioRenderBase() = default;
  virtual ~AudioRenderBase() {}
  virtual int Open() = 0;   // 打开音频播放设备并初始化
  virtual int Close() = 0;  // 关闭音频播放
  virtual int Clear() = 0;  // 清除缓存, 恢复初始状态
  virtual uint64_t
  GetUnplayedDuration() = 0;  // 获取当前还有多少ms的音频数据在缓区没有播放
  virtual int Write(
      std::vector<uint8_t> const& pcm_data) = 0;  // 向缓冲区写入pcm数据
  void SetVolume(double volume) { this->volume = volume; };
  virtual int Suspend() = 0;            // 暂停
  virtual int Resume() = 0;             // 恢复
  virtual int GetFreeBufferSize() = 0;  // 可写入数据的空间
};

class VideoRenderBase {
 public:
  VideoRenderBase() = default;
  virtual ~VideoRenderBase() {}
  virtual void Clear() = 0;
  virtual void Init(std::string const& title, uint32_t width, uint32_t height,
                    void* windowHandle) = 0;  // 初始化窗口宽高
  virtual void Render(
      std::shared_ptr<Frame> pFrame) = 0;  // 渲染一帧画面，并维持duration ms
  virtual void ResetWindow(int _x,int _y,int _w,int _h) = 0;  // 修改渲染区域的位置和大小
};

}  // namespace AV

class QIODevice;
class QMediaDevices;
class QAudioDevice;
class QAudioSink;

class QtAudioRender final : public AV::AudioRenderBase {
  QAudioSink* pAudioSinkOutput_ = nullptr;
  QIODevice* pIoDevice_ = nullptr;
  QMediaDevices* pMediaDevice_ = nullptr;
  QAudioDevice* pAudioDevice_;

 public:
  QtAudioRender() = default;
  virtual ~QtAudioRender() = default;
  virtual uint64_t GetUnplayedDuration() override;
  virtual int Write(std::vector<uint8_t> const& pcm_data) override;
  virtual int Clear() override;
  virtual int Close() override;
  virtual int Open() override;
  virtual int Suspend() override;
  virtual int Resume() override;
  virtual int GetFreeBufferSize() override;
};

class QImage;

class QtVideoRender final : public AV::VideoRenderBase {
 public:
  QtVideoRender() = default;
  virtual ~QtVideoRender() = default;
  virtual void Clear() override;
  virtual void Init(std::string const& title, uint32_t width, uint32_t height,
                    void* windowHandle) override;  // 初始化窗口宽高
  virtual void Render(std::shared_ptr<AV::Frame> pFrame)
      override;  // 渲染一帧画面，并维持duration ms

 private:
  // QImage* FrameToQImage(std::shared_ptr<Frame> pFrame);
  // struct SwsContext* pImgConvertContext_;
  // AVFrame* pDstFrame_;
};

struct SDL_Window;
struct SDL_Rect;
struct SDL_Renderer;
struct SDL_Texture;

class SDLVideoRender final : public AV::VideoRenderBase {
	//为了降低反复创建部分结构体产生的空间和时间花费，强烈建议对于每一个播放任务只调用一次Render
	//因为内部的反复调用会导致高码率的视频丢帧
 public:
  SDLVideoRender();
  virtual ~SDLVideoRender();
  virtual void Clear() override;
  virtual void Init(std::string const& title, uint32_t width, uint32_t height,
                    void* windowHandle) override;  // 初始化窗口宽高
  virtual void Render(
      std::shared_ptr<AV::Frame> pFrame) override;  // 渲染一帧画面
  virtual void ResetWindow(int _x,int _y,int _w,int _h) override;

  void ResizeVideo_FullScreen();
  void ResizeVideo(int _w,int _h);

  void ResizeWindow();
  void ClearScreen();

 private:
  inline void RenderOnWindowFromTexture();
  int VideoRenderWidth,VideoRenderHeight;
  int WindowHeight,WindowWidth;
  SDL_Window* pWindow_;
  SDL_Rect* pDrawRect_;
  SDL_Renderer* pRender_;
  SDL_Texture* pTexture_;
  AV::Converter* pConverter_;
};
