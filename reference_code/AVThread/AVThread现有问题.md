
未解决的bug

bug3: (QT的报错，只是用了他的几个类) 没有发现影响
```shell
QObject::startTimer: Timers can only be used with threads started with QThread
QObject::killTimer: Timers cannot be stopped from another thread
QObject::startTimer: Timers can only be used with threads started with QThread
QObject::killTimer: Timers cannot be stopped from another thread
QObject::startTimer: Timers can only be used with threads started with QThread
```

bug4:
```shell 
[h264 @ 000001eb52df3280] co located POCs unavailable
IMPT: [D:\SoftwareEngineeringLab\qt-video-player\Kernel\AVThread\VideoRenderThread.cpp:41->RunOnce] video pts = 9223372036854775808, sync pts = 30
```

bug7: 会导致视频渲染停止，随机概率发生
```shell
[h264 @ 000001b78cad3340] reference picture missing during reorder
[h264 @ 000001b78cad3340] Missing reference picture, default is 6
[h264 @ 000001b78cad3340] mmco: unref short failure
```