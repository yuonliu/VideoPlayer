/**
 * @author yilongdong (dyl20001223@163.com)
 * @date 2022-04-27
 * @details 封装mediaplayer的各个线程，对外提高统一的控制接口
 * 目前有的功能是：
 *  正常播放，暂停，恢复，随机寻址，设置倍速，音画同步，音频波形数据，下一帧，获取视频详细信息
 *  音频波形数据，下一帧，获取视频详细信息，
 *  其中音频波形数据，关闭还没有经过测试
 * @version 0.1
 * @copyright Copyright (c) 2022
 * TODO: 很糟糕，需要大幅度重构
 */

#pragma once
#include "AVThread/AVThread.h"

namespace AV {

// 视频倒放建议在播放器上层使用 ffmpeg -i xxx.mp4 -vf reverse -af areverse
// xxx_r.mp4 实现
// 先简单实现了再说，之后可以采用视频切分，局部翻转，实现较为实时的倒放

class MediaPlayer {
 public:
  MediaPlayer();

  ~MediaPlayer();

  void Open(std::string const& filename, void* windowHandle);

  void Play();

  void Pause();

  void Close();

  void Seek(double percent);

  void NextFrame();

  // 音量过大过小会被截断，位深只有16bit
  void SetSpeed(double speed_rate);

  void SetVolume(double volume);

  void SetPCMQueue(std::shared_ptr<AV::PCMQueue> pPCMQueue);

  void ResetWindow(int _x,int _y,int _w,int _h);

  // void SetReverseMode();
  AV::VideoInfo GetVideoInfo() const;

 private:
  void _Init();

 private:
  std::shared_ptr<AV::PacketQueue> pAudioPacketQueue_;
  std::shared_ptr<AV::PacketQueue> pVideoPacketQueue_;
  std::shared_ptr<AV::FrameQueue> pAudioFrameQueue_;
  std::shared_ptr<AV::FrameQueue> pVideoFrameQueue_;

  std::shared_ptr<AV::AudioVideoSynchronizer> pAVSynchronizer_;

  std::shared_ptr<AV::PCMQueue> pPCMQueue_;

  // std::unique_ptr<AV::DemuxThread> pDemuxThread_;
  // std::unique_ptr<AV::DecodeThread> pAudioDecodeThread_;
  // std::unique_ptr<AV::DecodeThread> pVideoDecodeThread_;
  // std::unique_ptr<AV::VideoRenderThread> pVideoRenderThread_;
  // std::unique_ptr<AV::AudioRenderThread> pAudioRenderThread_;
  AV::DemuxThread oDemuxThread_;
  AV::DecodeThread oAudioDecodeThread_;
  AV::DecodeThread oVideoDecodeThread_;
  AV::VideoRenderThread oVideoRenderThread_;
  AV::AudioRenderThread oAudioRenderThread_;
  std::thread checkThread_;
  bool onlyAudio_ = false;

  mutable std::mutex mutex_;
};

}  // namespace AV
