#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <numeric>
#include <queue>


template<typename T, typename ConQueue = std::queue<T>>
class ConcurrentQueue
{
public:
	using mutex_type = std::mutex;
	using value_type = T;

public:
	ConcurrentQueue(uint32_t capacity);
	~ConcurrentQueue() = default;

	void push(T const& value);
	void pop(T& value);

	bool push_timeout(T const& value, uint32_t timeout);
	bool pop_timeout(T& value, uint32_t timeout);

	bool try_push(T const& value);
	bool try_pop(T& value);
	
	uint32_t size() const;
	uint32_t getCapacity() const;
	void clear();

private:
	ConQueue us_queue;
	uint32_t capacity;

	mutable mutex_type mtx;
	std::condition_variable cv_empty;
	std::condition_variable cv_full;

};

template<typename T,typename ConQueue>
ConcurrentQueue<T, ConQueue>::ConcurrentQueue(uint32_t capa)
	:capacity(capa) {}



template<typename T, typename ConQueue>
void ConcurrentQueue<T, ConQueue>::push(T const& value)
{
	std::unique_lock<mutex_type> locker(mtx);
	cv_full.wait(locker, [this]->{return us_queue.size() < capacity; });
	us_queue.push(value);
	cv_empty.notify_one();
}

template<typename T, typename ConQueue>
void ConcurrentQueue<T, ConQueue>::pop(T& value)
{
	std::unique_lock<mutex_name> locker(mtx);
	cv_empty.wait(locker, [this]->{return !us_queue.empty(); });
	value = std::move(us_queue.front());
	us_queue.pop();
	cv_full.notify_one();
}

template <typename T, typename ConQueue>
bool ConcurrentQueue<T, ConQueue>::try_push(T const& value)
{
	std::unique_lock < mutex_type locker(mtx);
	if (us_queue.size() == capacity) return false;
	us_queue.push(value);
	cv_empty.notify_all();
	return true;
}

template <typename T, typename ConQueue>
bool ConcurrentQueue<T, ConQueue>::try_pop(T& value) 
{
	std::unique_lock<mutex_type> locker(mtx);
	if (us_queue.empty()) return false;
	value = std::move(us_queue.front());
	us_queue.pop();
	cv_full.notify_all();
	return true;
}

template <typename T,typename ConQueue>
uint32_t ConcurrentQueue<T, ConQueue>::getCapacity() const
{
	return capacity;
}

template <typename T,typename ConQueue>
bool ConcurrentQueue<T, ConQueue>::push_timeout(T const& value, uint32_t timeout)
{
	std::unique_lock<mutex_type> locker(mtx);
	if (cv_full.wait_for(locker,
		std::chrono::milliseconds(timeout),
		[this]->{return us_queue.size() < capacity; })) {

		us_queue.push(value);
		cv_empty.notify_all();
		return true;
	}
	return false;
}

template <typename T, typename ConQueue>
bool ConcurrentQueue<T, ConQueue>::pop_timeout(T& value, uint32_t timeout)
{
	std::unique_lock<mutex_type> locker(mtx);
	if (cv_empty.wait_for(locker,
		std::chrono::milliseconds(timeout),
		[this]->{return !us_queue.empty(); })) {
		value = us_queue.front();
		us_queue.pop();
		cv_full.notify_all();
		return true;
	}
	return false;
}

