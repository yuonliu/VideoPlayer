/**
 * @file main.cpp
 * @author yilongdong (dyl20001223@163.com)
 * @brief test
 * @details
 * 目前有的功能是：
 *  正常播放，暂停，恢复，随机寻址，设置倍速，音画同步，
 *  同步后的音频波形数据，下一帧，获取视频详细信息，
 *  其中音频波形数据，关闭还没有经过测试
 * @version 0.1
 * @date 2022-04-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#define QT_NO_DEBUG_OUTPUT
#define QT_NO_INFO_OUTPUT
#define QT_NO_WARNING_OUTPUT
#include <gflags/gflags.h>

#include <cstdlib>
#include <iostream>

#include "AVPlayer/AVPlayer.h"
DEFINE_string(
    filename,
    "D:/SoftwareEngineeringLab/qt-video-player/Kernel/Static/踏浪高歌.mp4",
    "filename");

int main(int argc, char* argv[]) {
  // chcp 65001是一句cmd指令，用来把终端的活动页(So called "Code
  // Page")切换成UTF-8。源文件的编码是UTF-8 CRLF
  std::system("chcp 65001");
  // SetConsoleOutputCP(65001); // #include <windows.h>
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // "D:/SoftwareEngineeringLab/qt-video-player/Kernel/Static/踏浪高歌.mp4"
  // "D:/SoftwareEngineeringLab/qt-video-player/Kernel/Static/蜕变的稚气.mp4"
  // "D:/SoftwareEngineeringLab/qt-video-player/Kernel/Static/华工之歌.mp3"

  AV::MediaPlayer player;
  LOGIMPT("open");
  player.Open(FLAGS_filename.c_str(), nullptr);
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  LOGIMPT("video info");
  auto videoinfo = player.GetVideoInfo();  // 获取音频数据还有问题
  for (auto const& [key, value] : videoinfo) {
    LOGIMPT("key: %s, value: %s", key.c_str(), value.c_str());
  }
  LOGIMPT("play");
  player.Play();
  player.ResetWindow(200, 100, 1920, 1080);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  player.ResetWindow(200, 100, 1440, 810);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  LOGIMPT("pause");
  player.Pause();
  player.ResetWindow(200, 100, 1280, 720);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  LOGIMPT("play");
  player.Play();
  std::this_thread::sleep_for(std::chrono::seconds(10));
  LOGIMPT("set volume");
  player.SetVolume(2.5);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  player.Play();
  LOGIMPT("seek 0.5");
  player.Seek(0.5);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  LOGIMPT("speed = 2");  // 除了变速时有点跑调其他都还行233
  // 变速时高码率的视频播放存在卡顿现象
  player.SetSpeed(2);
  std::this_thread::sleep_for(std::chrono::seconds(30));
  return 0;
}
