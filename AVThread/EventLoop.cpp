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

	}
}