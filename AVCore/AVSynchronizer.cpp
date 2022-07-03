#include "AVCore.h"

AVSynchronizer::AVSynchronizer():syncPts(0) {}

uint64_t AVSynchronizer::getSyncPts() {
	std::unique_lock<std::mutex> locker(mtx);
	return syncPts;
}

void AVSynchronizer::setSyncPts(uint64_t syncpts)
{
	std::lock_guard<std::mutex> locker(mtx);
	syncPts = syncpts;
}

void AVSynchronizer::suspend(){}

void AVSynchronizer::resume(){}

void AVSynchronizer::clear(){}

void AVSynchronizer::setSpeedRate(double rate){}

bool AVSynchronizer::shouldDiscardVideoFrame(uint64_t video_frame_pts)
{
	uint64_t sync_pts = getSyncPts();
	if (video_frame_pts > sync_pts && video_frame_pts - sync_pts > 4000) {
		return true;
	}
	return (video_frame_pts < sync_pts&& sync_pts - video_frame_pts > 200);
}

uint32_t AVSynchronizer::VideoFrameDelayDuration(uint64_t video_frame_pts)
{
	uint64_t sync_pts = getSyncPts();
	if (sync_pts < video_frame_pts) {
		return static_cast<uint32_t>(video_frame_pts - sync_pts);
	}
	return 0;
}

bool AVSynchronizer::shouldDiscardAudioFrame(uint64_t audio_frame_pts)
{
	uint64_t sync_pts = getSyncPts();
	return (audio_frame_pts < sync_pts&& sync_pts - audio_frame_pts > 100);
}

uint32_t AVSynchronizer::AudioFrameDelayDuration(uint64_t audio_frame_pts)
{
	uint64_t sync_pts = getSyncPts();
	if (sync_pts < audio_frame_pts && audio_frame_pts - sync_pts > 100) {
		return static_cast<uint32_t>(audio_frame_pts - sync_pts);
	}
	return 0;
}
