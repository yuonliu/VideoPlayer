/**
 * @file ThreadEventLoop.h
 * @author yilongdong (dyl20001223@163.com)
 * @date 2022-04-27
 * @brief 线程循环基类
 * @details
 * 作为具体线程类的接口，提供状态控制模板。其派生类需要实现RunOnce()，必要时可以实现suspend
 * resume等
 * @version 0.1
 * @copyright Copyright (c) 2022
 * TODO: 没有经过测试
 */
#pragma once
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace AV {

class ThreadEventLoop {
  enum class state : int {
    idle = 0,
    running = 1,
    suspended = 2,
    termination = 3
  };

 public:
  ThreadEventLoop();
  virtual ~ThreadEventLoop();

  void StartLoop();
  void ExitLoop();
  void Suspend();
  void Resume();

  std::string GetName() const;
  void SetName(std::string const&);
  int Clear();
  std::string CurState() const;

 protected:
  void Finish();  // 和在RunOnce中返回非0值差不多效果。都置线程状态为终止态
  virtual int RunOnce() = 0;
  void BeforeDerivedClassDeconstruct();

 private:
  /* non-virtual interface(NVI)手法:令用户通过public
   * non-virtual成员函数间接调用private
   * virtual函数，将这个non-virtual函数称为virtual函数的wrapper.
   * wrapper确保得以在一个virtual函数被调用之前设定好适当场景，并在调用结束之后清理场景
   */
  virtual int ClearImpl() = 0;
  virtual int SuspendImpl() { return 0; };
  virtual int ResumeImpl() { return 0; };
  virtual int BeforeRun() { return 0; };

 protected:
  mutable std::mutex
      mutex_;  // 负责RunOnce和Clear的互斥，以防止RunOnce了一半clear了
  std::atomic<ThreadEventLoop::state> state_flag_;

 private:
  int Run();

  std::thread thread_;
  std::string thread_name_;
};
}  // namespace AV