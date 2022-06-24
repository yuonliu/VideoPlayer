#include "AVCore.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
}

namespace AV {
Demuxer::Demuxer() {
  LOGIMPT("construct Demuxer");
  formatCtx = avformat_alloc_context();
  avformat_network_init();
  LOGERROR_IF(!formatCtx, "alloc format context fail!");
}

Demuxer::~Demuxer() {
  LOGIMPT("deconstruct Demuxer");
  Demuxer::Clear();
  if (formatCtx) {
    avformat_free_context(formatCtx);
    formatCtx = nullptr;
  }
}

int Demuxer::Open(std::string const& filename) {
  LOGERROR_IF(!formatCtx, "Open fail. because format context is nullptr");
  if (formatCtx == nullptr) return -1;
  int ret = avformat_open_input(&formatCtx, filename.data(), nullptr, nullptr);
  LOGERROR_IF(ret, "avformat open input fail %s", filename.c_str());

  if (ret == 0) {
    avformat_find_stream_info(formatCtx, nullptr);
    // 打印视频流详细信息
    av_dump_format(formatCtx, 0, filename.data(), 0);
  }

  infoMap_.emplace("filename", filename);
  _GetInfo();

  for (auto const& [key, value] : infoMap_) {
    LOGINFO("key = %s, value = %s", key.c_str(), value.c_str());
  }

  totalDuration_ =
      static_cast<uint32_t>(formatCtx->duration / (AV_TIME_BASE / 1000));
  LOGINFO("视频总时长 = %lu", totalDuration_);
  return ret;
}

int Demuxer::Close() {
  LOGIMPT("close demuxer");
  return Reset();
}

int Demuxer::Reset() {
  if (formatCtx == nullptr) return -1;
  avformat_close_input(&formatCtx);
  totalDuration_ = 0;
  return 0;
}

int Demuxer::Clear() {
  if (formatCtx == nullptr) return -1;
  // 清理读取缓冲
  avformat_flush(formatCtx);
  return 0;
}

int Demuxer::Read(std::shared_ptr<Packet> pPacket) {
  LOGERROR_IF(!formatCtx, "format context is nullptr!");
  int ret = av_read_frame(formatCtx, pPacket->pkt);
  if (ret == AVERROR_EOF) return 1;
  if (ret) {
    LOGERROR("read frame error. ret = %d", ret);
  }
  // 把pts改成ms为单位
  pPacket->pkt->pts = static_cast<int64_t>(
      pPacket->pkt->pts *
      (1000 *
       av_q2d(formatCtx->streams[pPacket->pkt->stream_index]->time_base)));
  return ret;
}

uint64_t Demuxer::Seek(double percent) {
  LOGERROR_IF(formatCtx == nullptr, "formatCtx is nullptr");
  avformat_flush(formatCtx);  // 清空读取缓存
  int stream_index = -1;
  if (onlyAudio_) {
    stream_index = GetAudioStreamIndex();
  } else {
    stream_index = GetVideoStreamIndex();
  }
  int64_t seek_pos = static_cast<int64_t>(
      formatCtx->streams[stream_index]->duration * percent);
  // AVSEEK_FLAG_BACKWARD 允许自动往回seek到最近的关键帧
  av_seek_frame(formatCtx, stream_index, seek_pos,
                AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
  return static_cast<uint64_t>(percent * totalDuration_);
}

int Demuxer::GetStreamCount() { return formatCtx->nb_streams; }

AVCodecParameters* Demuxer::GetStreamCodecPar(uint32_t index) {
  if (index >= formatCtx->nb_streams) return nullptr;
  AVCodecParameters* codecpar = avcodec_parameters_alloc();
  avcodec_parameters_copy(codecpar, formatCtx->streams[index]->codecpar);
  return codecpar;
}

int Demuxer::GetVideoStreamIndex() {
  if (onlyAudio_) {
    LOGIMPT("only audio");
    return -1;
  }
  return av_find_best_stream(formatCtx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1,
                             nullptr, 0);
}

int Demuxer::GetAudioStreamIndex() {
  return av_find_best_stream(formatCtx, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1,
                             nullptr, 0);
}

std::map<std::string, std::string> Demuxer::GetVideoInfo() const {
  return infoMap_;
}

// 参考https://blog.csdn.net/qq_42780025/article/details/113931002
double Demuxer::_GetInfo() {
  LOGERROR_IF(formatCtx == nullptr, "formatCtx is nullptr");
  // 视频流数量
  infoMap_.emplace("stream_numbers", std::to_string(formatCtx->nb_streams));

  // 视频时长
  int hours, mins, secs;
  secs = formatCtx->duration / 1000000;
  mins = secs / 60;
  secs %= 60;
  hours = mins / 60;
  mins %= 60;
  char duration_foramt_[128];
  sprintf(duration_foramt_, "%d:%d:%d", hours, mins, secs);
  infoMap_.emplace("duration", duration_foramt_);

  // 比特率
  infoMap_.emplace("bit rate", std::to_string(formatCtx->bit_rate));

  // 封装格式
  infoMap_.emplace("format", formatCtx->iformat->name);

  AVDictionaryEntry* tag = nullptr;
  while ((
      tag = av_dict_get(formatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
    infoMap_.emplace(tag->key, tag->value);
  }

  for (uint32_t i = 0; i < formatCtx->nb_streams; ++i) {
    AVStream* input_stream = formatCtx->streams[i];
    if (input_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      // 视频帧率
      if (input_stream->avg_frame_rate.den == 0) {
        // 说明是单纯的音频文件，此时直接跳过视频信息解析
        onlyAudio_ = true;
        continue;
      }
      infoMap_.emplace("frame_rate",
                       std::to_string(input_stream->avg_frame_rate.num /
                                      input_stream->avg_frame_rate.den));

      AVCodecParameters* codec_par = input_stream->codecpar;

      // 利用编码参数对象AVCdecParamters得到视频宽度，高度，码率
      infoMap_.emplace("width", std::to_string(codec_par->width));
      infoMap_.emplace("height", std::to_string(codec_par->height));
      infoMap_.emplace("video average bit rate",
                       std::to_string(codec_par->bit_rate));

      AVCodecContext* avctx_video;
      avctx_video = avcodec_alloc_context3(NULL);
      int ret = avcodec_parameters_to_context(avctx_video, codec_par);
      if (ret < 0) {
        avcodec_free_context(&avctx_video);
        return 0;
      }
      char buf[128];
      avcodec_string(buf, sizeof(buf), avctx_video, 0);
      infoMap_.emplace("video_format", avcodec_get_name(codec_par->codec_id));
    } else if (input_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      AVCodecParameters* codec_par = input_stream->codecpar;
      AVCodecContext* avctx_audio;
      avctx_audio = avcodec_alloc_context3(NULL);
      int ret = avcodec_parameters_to_context(avctx_audio, codec_par);
      if (ret < 0) {
        avcodec_free_context(&avctx_audio);
        return 0;
      }

      infoMap_.emplace("audio format", avcodec_get_name(avctx_audio->codec_id));
      infoMap_.emplace("audio average bit rate",
                       std::to_string(codec_par->bit_rate));
      infoMap_.emplace("channel nums", std::to_string(codec_par->channels));
      infoMap_.emplace("sample rate", std::to_string(codec_par->sample_rate));
    }
  }

  return 0;
}

}  // namespace AV