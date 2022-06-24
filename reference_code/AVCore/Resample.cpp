
#include <iostream>

#include "AVCore.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

namespace AV {

Resample::Resample() {
  swrContext_ = swr_alloc();
  LOGERROR_IF(!swrContext_, "swr_alloc context fail");
  LOGIMPT("construct resample");
}

Resample::~Resample() {
  if (swrContext_) {
    swr_free(&swrContext_);
    swrContext_ = nullptr;
  }
  LOGIMPT("deconstruct resample");
}

int Resample::Close() {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGIMPT("resample close");
  if (swrContext_) {
    swr_free(&swrContext_);
    swrContext_ = nullptr;
  }
  return 0;
}

int Resample::Open(AVCodecParameters* para) {
  std::unique_lock<std::mutex> locker(mutex_);
  codecPara_ = para;

  LOGINFO("Resample Open");
  LOGERROR_IF(!para, "AVCodecParameters is nullptr");
  LOGONTEST("Set Context Options");
  swrContext_ = swr_alloc_set_opts(
      swrContext_, av_get_default_channel_layout(para->channels),
      /* 输出格式, 固定为双声道 */ (AVSampleFormat)outputFormat_,
      /* 输出样本格式 AV_SAMPLE_FMT_S16 */
      para->sample_rate, /* 输出采样率，一秒钟音频样本数量
                          */
      av_get_default_channel_layout(para->channels),
      /*输入格式*/ (AVSampleFormat)para->format,
      /* 输入样本格式 */ para->sample_rate, /* 输入采样率 */
      0, 0                                  /* 日志 */
  );
  channel_num = para->channels;
  LOGONTEST("Init Context");
  int ret = swr_init(swrContext_);
  LOGERROR_IF(ret, "Init resample context error! ret = %d", ret);
  return ret;
}

int Resample::GetOriginSampleRate() {
  std::unique_lock<std::mutex> locker(mutex_);
  return codecPara_->sample_rate;
}

int Resample::DoResample(std::shared_ptr<Frame> pFrame,
                         std::vector<uint8_t>& pcm) {
  std::unique_lock<std::mutex> locker(mutex_);
  pcm.resize(1024 * 1024 * 10);
  LOGERROR_IF(!pFrame || !pFrame->frame, "Frame ptr is nullptr. Please Check!");
  uint8_t* data[2] = {0};
  data[0] = pcm.data();
  data[1] = pcm.data();
  // nb_sample样本个数 * 样本大小(2字节，16比特)，通道数(双通道)
  LOGONTEST("Swr convert begin");
  LOGERROR_IF(swrContext_ == nullptr, "swrContext is nullptr");
  int number_of_sample_output_per_channel =
      swr_convert(swrContext_,                      // 重采样上下文
                  data, pFrame->frame->nb_samples,  // 输出数据和样本大小
                  (uint8_t const**)(pFrame->frame->data),
                  pFrame->frame->nb_samples  // 输入数据和样本大小
      );
  LOGWARN_IF(number_of_sample_output_per_channel < 0,
             "do resample error! ret = %d",
             number_of_sample_output_per_channel);
  LOGONTEST("Swr convert end");
  pcm.resize(number_of_sample_output_per_channel * pFrame->frame->channels *
             av_get_bytes_per_sample((AVSampleFormat)outputFormat_));

  // for(auto& digit : pcm) {
  // 	digit = 0.5 * digit;
  // }
  return 0;
}

int Resample::SetOutputFormat(uint32_t format) {
  std::unique_lock<std::mutex> locker(mutex_);
  outputFormat_ = format;
  LOGINFO("Set output format = %u", format);
  return 0;
}

int Resample::SetDefaultOutputFormat() {
  std::unique_lock<std::mutex> locker(mutex_);
  outputFormat_ = AV_SAMPLE_FMT_S16;
  LOGINFO("Set default output format = %u", outputFormat_);
  return 0;
}

int Resample::Clear() { return 0; }

int Resample::SetSpeedRate(double speed_rate) {
  std::unique_lock<std::mutex> locker(mutex_);
  LOGIMPT("set speed rate = %f", speed_rate);
  static double epsilon = 0.01;

  if (speed_rate < epsilon || std::abs(speed_rate - 1.0) < epsilon ||
      speed_rate < 0) {
    sample_rate = codecPara_->sample_rate;
  } else {
    sample_rate = 1.0 * codecPara_->sample_rate / speed_rate;
  }
  LOGINFO("origin sample rate = %d, cur sample rate = %d",
          codecPara_->sample_rate, sample_rate);
  if (swrContext_) {
    swr_free(&swrContext_);
    swrContext_ = nullptr;
  }

  swrContext_ = swr_alloc();
  LOGERROR_IF(swrContext_ == nullptr, "swr_alloc context fail");

  LOGERROR_IF(!codecPara_, "codecPara_ is nullptr");
  LOGONTEST("Set Context Options");
  swrContext_ = swr_alloc_set_opts(
      swrContext_, av_get_default_channel_layout(2),
      /* 输出格式, 固定为双声道 */ (AVSampleFormat)outputFormat_,
      /* 输出样本格式 AV_SAMPLE_FMT_S16 */
      sample_rate, /* 输出采样率，一秒钟音频样本数量
                    */
      av_get_default_channel_layout(codecPara_->channels),
      /*输入格式*/ (AVSampleFormat)codecPara_->format,
      /* 输入样本格式 */ codecPara_->sample_rate, /* 输入采样率 */
      0, 0                                        /* 日志 */
  );
  LOGONTEST("Init Context");
  int ret = swr_init(swrContext_);
  LOGERROR_IF(ret, "Init resample context error! ret = %d", ret);
  return 0;
}

}  // namespace AV