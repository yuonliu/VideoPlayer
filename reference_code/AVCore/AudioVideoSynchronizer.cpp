#include "AVCore.h"

namespace AV {

AudioVideoSynchronizer::AudioVideoSynchronizer() : sync_pts_(0) {
  // std::unique_lock<std::mutex> locker(mutex_);
  // is_suspended_ = true;
  // sync_pts_ = 0;
  // t1_ = t2_ = std::chrono::steady_clock::now();
  // speed_rate_ = 1.0;
}
uint64_t AudioVideoSynchronizer::GetSyncPts() {
  std::unique_lock<std::mutex> locker(mutex_);
  // if(is_suspended_) return sync_pts_;
  // t2_ = std::chrono::steady_clock::now();
  // double delta_pts = std::chrono::duration<double, std::milli>(t2_ -
  // t1_).count(); sync_pts_ += static_cast<uint64_t>(delta_pts * speed_rate_);
  // t1_ = t2_;
  return sync_pts_;
}
void AudioVideoSynchronizer::SetSyncPts(uint64_t syncpts) {
  std::lock_guard<std::mutex> locker(mutex_);
  sync_pts_ = syncpts;
}
void AudioVideoSynchronizer::Suspend() {
  // std::unique_lock<std::mutex> locker(mutex_);
  // if(is_suspended_) return;
  // is_suspended_ = true;
  // t2_ = std::chrono::steady_clock::now();
  // double delta_pts = std::chrono::duration<double, std::milli>(t2_ -
  // t1_).count(); sync_pts_ += static_cast<uint64_t>(delta_pts * speed_rate_);
}
void AudioVideoSynchronizer::Resume() {
  // std::unique_lock<std::mutex> locker(mutex_);
  // if(is_suspended_ == false) return;
  // is_suspended_ = false;
  // t1_ = t2_ = std::chrono::steady_clock::now();
}
void AudioVideoSynchronizer::Clear() {
  // std::unique_lock<std::mutex> locker(mutex_);
  // sync_pts_ = 0;
  // is_suspended_ = true;
  // t1_ = t2_ = std::chrono::steady_clock::now();
}
void AudioVideoSynchronizer::SetSpeedRate(double rate) {
  // std::lock_guard<std::mutex> locker(mutex_);
  // speed_rate_ = rate;
}
bool AudioVideoSynchronizer::ShouldDiscardVideoFrame(uint64_t video_frame_pts) {
  uint64_t sync_pts = GetSyncPts();
  if (video_frame_pts > sync_pts && video_frame_pts - sync_pts > 4000) {
    return true;  // 时间差过大就丢弃了
  }

  return (video_frame_pts < sync_pts && sync_pts - video_frame_pts > 200);
  // else if(video_frame_pts - sync_pts < 1000) {
  //   return false;
  // }·
  // else {
  //   LOGIMPT("invaild video frame pts = %llu, sync pts = %llu",
  //   video_frame_pts, sync_pts); return true;
  // }
}
uint32_t AudioVideoSynchronizer::VideoFrameDelayDuration(
    uint64_t video_frame_pts) {
  uint64_t sync_pts = GetSyncPts();
  if (sync_pts < video_frame_pts) {
    return static_cast<uint32_t>(video_frame_pts - sync_pts);
  }
  return 0;
}

bool AudioVideoSynchronizer::ShouldDiscardAudioFrame(uint64_t audio_frame_pts) {
  uint64_t sync_pts = GetSyncPts();
  return (audio_frame_pts < sync_pts && sync_pts - audio_frame_pts > 100);
}
uint32_t AudioVideoSynchronizer::AudioFrameDelayDuration(
    uint64_t audio_frame_pts) {
  uint64_t sync_pts = GetSyncPts();
  if (sync_pts < audio_frame_pts && audio_frame_pts - sync_pts > 100) {
    return static_cast<uint32_t>(audio_frame_pts - sync_pts);
  }
  return 0;
}
}  // namespace AV