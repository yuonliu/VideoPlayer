#include "ThreadEventLoop.h"

#include <unordered_map>

#include "Log/Log.h"

namespace AV {

ThreadEventLoop::ThreadEventLoop() {
  state_flag_ = state::idle;
  LOGONTEST("Thread: %s, Cur state is idle", GetName().c_str());
  LOGINFO_IF(!state_flag_.is_lock_free(),
             "Thread: %s, state flag is not lock free", GetName().c_str());
}

ThreadEventLoop::~ThreadEventLoop() {}

void ThreadEventLoop::StartLoop() {
  if (state_flag_ != state::idle) {
    LOGWARN(
        "Thread: %s, cur state is not state::idle. Call StartLoop() is "
        "useless.",
        GetName().c_str());
    return;
  }
  thread_ = std::move(std::thread(&ThreadEventLoop::Run, this));
  // 忘了为什么要sleep了......，之后看看什么用意当时
  // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  return;
}

void ThreadEventLoop::ExitLoop() {
  if (state_flag_ == state::termination) {
    LOGWARN("Thread: %s, ExitLoop() has been called.", GetName().c_str());
    return;
  }
  LOGIMPT("Thread: %s, Set state::termination", GetName().c_str());
  state_flag_ = state::termination;
  return;
}

void ThreadEventLoop::Suspend() {
  if (state_flag_ == state::termination) {
    LOGWARN(
        "Thread: %s, cur state is state::termination. Call Suspend() is "
        "useless.",
        GetName().c_str());
    return;
  }
  state_flag_ = state::suspended;

  std::unique_lock<std::mutex> locker(mutex_);
  SuspendImpl();
  return;
}

void ThreadEventLoop::Resume() {
  if (state_flag_ != state::suspended && state_flag_ != state::idle) {
    LOGWARN(
        "Thread: %s, cur state is not state::suspended or state::idle. Call "
        "Resume() is useless.",
        GetName().c_str());
    return;
  }
  state_flag_ = state::running;

  std::unique_lock<std::mutex> locker(mutex_);
  ResumeImpl();
  return;
}

void ThreadEventLoop::Finish() {
  // std::unique_lock<std::mutex> locker(mutex_); 重复加锁问题
  state_flag_ = state::termination;
}

int ThreadEventLoop::Run() {
  LOGIMPT("Thread: %s, Run.", GetName().c_str());
  {
    // 这个已经没有必要了，不过留下在方便以后扩展
    std::unique_lock<std::mutex> locker(mutex_);
    BeforeRun();
  }

  int ret = 0;
  for (;;) {
    LOGTRACE("Thread: %s, RunOnce. cur state = %d", GetName().c_str(),
             state_flag_.load());
    if (state_flag_ == state::idle) {
      // LOGTRACE("Thread: %s, cur state idle = %d", GetName().c_str(),
      // state_flag_.load());
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } else if (state_flag_ == state::suspended) {
      // LOGTRACE("Thread: %s, cur state suspended = %d", GetName().c_str(),
      // state_flag_.load());
      std::this_thread::sleep_for(std::chrono::milliseconds(10));

    } else if (state_flag_ == state::termination) {
      // LOGIMPT("Thread: %s, cur state termination = %d", GetName().c_str(),
      // state_flag_.load());
      break;
    } else if (state_flag_ == state::running) {
      // LOGTRACE("Thread: %s, cur state running = %d", GetName().c_str(),
      // state_flag_.load());
      std::unique_lock<std::mutex> locker(mutex_);
      ret = RunOnce();
      if (ret != 0) {
        state_flag_ = state::termination;
        return ret;
      }
    } else {
      LOGWARN("Thread: %s, unknown state!", GetName().c_str());
      break;
    }
  }
  LOGIMPT("Thread: %s, Run() has end", GetName().c_str());
  return ret;
}

void ThreadEventLoop::BeforeDerivedClassDeconstruct() {
  LOGIMPT("Before deconstruct");
  if (thread_.joinable()) {
    state_flag_ = state::termination;
    LOGIMPT("wait Thread: %s join.", GetName().c_str());
    // while(state_flag_.load() != state::termination) {
    //   LOGIMPT("Thread: %s, Try Set state::termination", GetName().c_str());
    //   state_flag_.store(state::termination);
    // }
    // LOGIMPT("Thread: %s, cur state termination = %d", GetName().c_str(),
    // state_flag_.load());
    thread_.join();
    LOGIMPT("Thread: %s, has join", GetName().c_str());
  }
  LOGIMPT("Thread %s has been joined", GetName().c_str());
}

std::string ThreadEventLoop::GetName() const { return thread_name_; }
void ThreadEventLoop::SetName(std::string const& name) { thread_name_ = name; }
int ThreadEventLoop::Clear() {
  std::unique_lock<std::mutex> locker(mutex_);
  int ret = ClearImpl();
  return ret;
}

std::string ThreadEventLoop::CurState() const {
  static std::unordered_map<ThreadEventLoop::state, std::string>
      state_to_state_name{
          {state::idle, "idle"},
          {state::running, "running"},
          {state::suspended, "suspended"},
          {state::termination, "termination"},
      };
  return state_to_state_name[state_flag_];
}
// 理想的方式
// FSMControl fsm;
// fsm.When(state::running, GetPacket);
// fsm.From(state::idle).To(state::running).On(event::play).Do(Init);
// fsm.From(state::running).To(state::paused).On(event::pause).Do();
// fsm.From(state::paused).To(state::running).On(event::play).Do();
// fsm.From(state::running).To(state::idle).On(event::stop).Do(Clear);
// fsm.From(state::paused).To(state::idle).On(event::stop).Do(Clear);
// std::thread([fsm]{ fsm.run(); }).detach();
}  // namespace AV