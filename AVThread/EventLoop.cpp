#include "EventLoop.h"

#include <iostream>
#include <unordered_map>

EventLoop::EventLoop() 
{
	state_flag = state::idle;
	std::cout << "Thread: " << getName() << " Current State is idle" << std::endl;
	if (!state_flag.is_lock_free()) {
		std::cout << "Thread: " << getName() << " State flag is not lock free" << std::endl;
	}
}

EventLoop::~EventLoop() {}

void EventLoop::startLoop()
{
	if (state_flag != state::idle) {
		std::cout << "Thread: " << getName() << "Currnet state is not idle, Call startLoop() failed." << std::endl;

		return;
	}
	//do not understand
	thread_ = std::move(std::thread(&EventLoop::run, this));

	return;
}

void EventLoop::exitLoop()
{
	if (state_flag == state::termination) {
		std::cout << "Thread: " << getName() << " ExitLoop() has been called." << std::endl;
		return;
	}
	std::cout << "Thread: " << getName() << " Set state::termination" << std::endl;
	state_flag = state::termination;
	return;
}

void EventLoop::suspend()
{
	if (state_flag == state::termination) {
		std::cout << "Thread: " << getName() << " Current state is termination, Call suspend() failed." << std::endl;
		return;
	}

	state_flag = state::suspended;
	std::unique_lock<std::mutex> locker(mtx);
	suspendImpl();
	return;
}

void EventLoop::resume()
{
	if (state_flag != state::suspended && state_flag != state::idle) {
		std::cout << "Thread: " << getName() << " Call resume() failed." << std::endl;
		return;
	}
	state_flag = state::running;
	std::unique_lock<std::mutex> locker(mtx);
	resumeImpl();
	return;
}

void EventLoop::finish()
{
	state_flag = state::termination;

}

int EventLoop::run()
{
	std::cout << "Thread: " << getName() << " run." << std::endl;
	{
		std::unique_lock<std::mutex> locker(mtx);
		beforeRun();
	}

	int ret = 0;
	for (;;) {

		std::cout << "Thread: " << getName() << " Current state: " <<
			static_cast<typename std::underlying_type<state>::type>(state_flag.load()) << std::endl;
		//typename std::underlying_type<state>::type = int

		if (state_flag == state::idle) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else if (state_flag == state::suspended) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else if (state_flag == state::termination) {
			break;
		}
		else if (state_flag == state::running) {
			std::unique_lock<std::mutex> locker(mtx);
			ret = runOnce();
			if (ret != 0) {
				state_flag = state::termination;
				return ret;
			}
		}
		else {
			std::cout << "Thread: " << getName() << " ,unknow state!" << std::endl;
			break;
		}
	}

	std::cout << "Thread: " << getName() << " run() has end" << std::endl;

	return ret;
}

std::string EventLoop::getName() const { return thread_name; }

void EventLoop::setName(std::string const& name) { thread_name = name; }

int EventLoop::clear() {
	std::unique_lock<std::mutex> locker(mtx);
	int ret = clearImpl();
	return ret;
}

std::string EventLoop::curState() const
{
	static std::unordered_map<state, std::string>
		state_to_string{
			{state::idle, "idle"},
			{state::running, "running"},
			{state::suspended, "suspended"},
			{state::termination, "termination"},
		};
	return state_to_string[state_flag];
}

