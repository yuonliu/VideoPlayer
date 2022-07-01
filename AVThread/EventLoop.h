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
	//����ɶ�ַ�������
/* non-virtual interface(NVI)�ַ�:���û�ͨ��public
 * non-virtual��Ա������ӵ���private
 * virtual�����������non-virtual������Ϊvirtual������wrapper.
 * wrapperȷ��������һ��virtual����������֮ǰ�趨���ʵ����������ڵ��ý���֮��������
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