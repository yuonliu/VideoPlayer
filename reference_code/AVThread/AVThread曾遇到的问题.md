记录一下AVThreadk开发过程中遇到了n多奇葩问题和趟过的坑，用以留念

1. 多线程下对象生命周期，一个线程已经析构了对象，另一个线程还在用orz，或者一个对象还在new，另一个线程就已经在使用了
2. 一个线程进行RunOnce， RunOnce了一半，另一个线程因为一些事情调用了一些对象的Clear等等
3. Seek操作需要很多流程，然后流程中间别的线程又调了了其他api，状态瞬间不对了。
4. 一个线程自己join了自己，然后死锁被操作系统杀死啦（背景是为了解决生命周期问题，自己搞了些CRTP和智能指针的奇怪操作，用shared_ptr管理资源，结果资源生命周期是解决了，但是最后一个持有这个资源的就是这个线程自己，然后自己join自己抛出系统异常）后来换了朴素实用的方式资源管理解决的。
5. 一个锁在线程中被锁了两次，一些没考虑到的地方会重复上锁。比如AVThread的基类GetName()这个函数。比如消息总线MessageBus在通知处理回调函数里面继续调用新的通知（异步回调自然也会面临依赖的资源生命周期是不是已经结束的问题）。
6. AVFrame AVPacket内存泄漏（解决方法是用C++封装了一层分配和释放，然后用智能指针管理）
7. 播放器在Stop时，给每个线程置标志位。结果就有一种很巧的情况。解复用线程被stop，并且停止了生产packet，此时音频packet队列刚好没数据了，然后又恰好音频解码线程正在RunOnce并且在阻塞的从音频packet队列取数据。但是解复用线程停下了，packet队列也没数据，所以注定取不到，就会一直等待。所以就不会执行到CheckState的流程不会退出，此时RunOnce又持有的锁，另一个尝试Stop这个线程的线程也会因为没有锁一直阻塞下去 orz。所以必须引入超时机制来解决。
8. 还有一些问题自己在命令行编译release版本不会暴漏，但是debug版本就出现了。比如提到的地7个问题。
9. 如果有抛出异常的话会导致程序执行路径比较特殊，此时的线程安全处理就很麻烦。
10. 现在的程序没太关心线程安全。等把主流程跑通我再重新写写 ♪(^∇^*)。


bug5:
进程结束时会提升5个调用std::abort()的C++RuntimeLibrary弹窗orz，大概是一个子线程一个的样子。说明程序结束时很多地方不太妙。 在调用Finish时一个线程重复加锁导致的

bug1: (在增加倍速功能时遇到的问题) 严重错误
```shell
[aac @ 000001f39d242ec0] Pulse data corrupt or invalid.
[aac @ 000001f39d242ec0] Number of bands (26) exceeds limit (8).
[aac @ 000001f39d242ec0] Number of scalefactor bands in group (51) exceeds limit (49).
[aac @ 000001f39d242ec0] Prediction is not allowed in AAC-LC.
[aac @ 000001f39d242ec0] Number of bands (25) exceeds limit (4).
[aac @ 000001f39d242ec0] Number of bands (8) exceeds limit (6).
[aac @ 000001f39d242ec0] Prediction is not allowed in AAC-LC.
[aac @ 000001f39d242ec0] Prediction is not allowed in AAC-LC.
[aac @ 000001f39d242ec0] Number of bands (25) exceeds limit (6).
```
bug2: (在增加倍速功能时遇到的问题) 严重错误
```shell
[aac @ 0000023bd5fd2ec0] Gain control is not implemented. Update your FFmpeg version to the newest one from Git. If the problem still occurs, it means that your file has a feature which has not been implemented.
[aac @ 0000023bd5fd2ec0] Inconsistent channel configuration.
[aac @ 0000023bd5fd2ec0] get_buffer() failed
```
bug1和bug2会导致在重采样时有概率出现读空指针异常，并且声音质量下降有zao's。出现在ffmepg对应库实现的内部，是个dll文件，没有源码不怎么好debug。
啊啊啊啊啊啊我蠢死了，原来是把audio的数据放到video里面去了，然后debug de了一天！！啊啊啊好蠢。

bug: 在vscode下编译运行会出现音质很差的问题，但是在Windows Termianl下却音质很好。
解决: 仔细debug后发现是日志打多了，vscode对于输出的优化做的不是很好，所以会卡顿。更改日志等级可以解决问题。
