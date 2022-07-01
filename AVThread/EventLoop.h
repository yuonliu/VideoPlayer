#pragma once


#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class EventLoop
{
	enum class state {
		idle = 0,
		running = 1,
		suspended = 2,
		termination = 3
	};

public:
	EventLoop();
	virtual ~EventLoop();

	void startLoop();
	void exitLoop();
	void suspend();
	void resume();

	std::string getName() const;
	void setName(std::string const&);
	int clear();
	std::string curState() const;

protected:
	void Finish();
	virtual int RunOnce() = 0;
	void beforeDerivedClassDestruct();

private:
	//这是啥手法？不懂
/* non-virtual interface(NVI)手法:令用户通过public
 * non-virtual成员函数间接调用private
 * virtual函数，将这个non-virtual函数称为virtual函数的wrapper.
 * wrapper确保得以在一个virtual函数被调用之前设定好适当场景，并在调用结束之后清理场景
 */
	virtual int clearImpl() = 0;
	virtual int suspendImpl() { return 0; };
	virtual int resumeImpl() { return 0; };
	virtual int beforeRun() { return 0; };

protected:
	mutable std::mutex mtx;
	std::atomic<EventLoop::state> state_flag;

private:
	int run();
	std::thread thread_;
	std::string thread_name;
};