#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QMediaDevices>
#include <unordered_map>

#include "AVCore.h"
#include "Log/Log.h"

uint64_t QtAudioRender::GetUnplayedDuration() {
  LOGERROR_IF(pAudioSinkOutput_ == nullptr, "AudioSinkOutput_ is nullptr");
  double unusedSize =
      pAudioSinkOutput_->bufferSize() - pAudioSinkOutput_->bytesFree();
  LOGTRACE("unused buffer size = %f", unusedSize);
  //一秒音频字节大小
  double secSize = sampleRate * (sampleSize / 8.0) * channels;
  uint64_t unplayedDuration =
      (unusedSize / secSize) * 1000 > 0 ? (unusedSize / secSize) * 1000 : 0;
  LOGTRACE("unplayed duration = %llu", unplayedDuration);
  return unplayedDuration;  // 毫秒
}
int QtAudioRender::Write(std::vector<uint8_t> const& pcm_data) {
  LOGTRACE("pcm_data.size() = %zu", pcm_data.size());
  int16_t* pSample = (int16_t*)pcm_data.data();
  int sample_num = pcm_data.size() / sizeof(int16_t);
  for (int i = 0; i < sample_num; ++i) {
    double sample_value = pSample[i] * volume;  // volume 来自 AudioRenderBase

    // 音量过大或者过小时做个截断
    if (sample_value < std::numeric_limits<int16_t>::min()) {
      pSample[i] = std::numeric_limits<int16_t>::min();
    } else if (sample_value > std::numeric_limits<int16_t>::max()) {
      pSample[i] = std::numeric_limits<int16_t>::max();
    } else {
      pSample[i] = (int16_t)sample_value;
    }
  }
  pIoDevice_->write((char const*)pcm_data.data(), pcm_data.size());
  if (pPCMQue) {
    pPCMQue->try_push(pcm_data);
  }
  return 0;
}
int QtAudioRender::Clear() {
  LOGTRACE("io deviec reset");
  pIoDevice_->reset();
  return 0;
}
int QtAudioRender::Close() {
  LOGTRACE("Close");
  pIoDevice_->close();
  pIoDevice_ = nullptr;
  pAudioSinkOutput_->stop();
  delete pAudioSinkOutput_;
  pAudioSinkOutput_ = nullptr;
  delete pMediaDevice_;
  pMediaDevice_ = nullptr;
  return 0;
}
int QtAudioRender::Open() {
  LOGIMPT("sample_rate = %d, sample_size = %d, channels = %d", sampleRate,
          sampleSize, channels);
  std::unordered_map<int, QAudioFormat::ChannelConfig> channel_num_to_format{
      {1, QAudioFormat::ChannelConfigMono},
      {2, QAudioFormat::ChannelConfigStereo},
      {3, QAudioFormat::ChannelConfig2Dot1},
      {5, QAudioFormat::ChannelConfigSurround5Dot0},
      {6, QAudioFormat::ChannelConfigSurround5Dot1},
      {7, QAudioFormat::ChannelConfigSurround7Dot0},
      {8, QAudioFormat::ChannelConfigSurround7Dot0}};

  QAudioFormat audioFormat;
  audioFormat.setSampleRate(sampleRate);
  audioFormat.setChannelConfig(channel_num_to_format[channels]);
  audioFormat.setSampleFormat(QAudioFormat::Int16);
  LOGERROR_IF(QMediaDevices::defaultAudioOutput().isFormatSupported(
                  audioFormat) == false,
              "this default audio device not supported the format");
  pAudioSinkOutput_ =
      new QAudioSink(QMediaDevices::defaultAudioOutput(), audioFormat);
  pIoDevice_ = pAudioSinkOutput_->start();
  return 0;
}

int QtAudioRender::Suspend() {
  LOGIMPT("Suspend");
  pAudioSinkOutput_->suspend();
  return 0;
}

int QtAudioRender::Resume() {
  LOGIMPT("Resume");
  pAudioSinkOutput_->resume();
  return 0;
}

int QtAudioRender::GetFreeBufferSize() {
  return pAudioSinkOutput_->bytesFree();
  return 0;
}