#ifndef _MELON_TIMER_MANAGER_H_
#define _MELON_TIMER_MANAGER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Timestamp.h"

#include <map>
#include <memory>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>

namespace melon {

class Processer;
class Timer {
public:
	typedef std::shared_ptr<Timer> Ptr;
	Timer(Timestamp timestamp, Processer* processer, Coroutine::Ptr coroutine, uint64_t interval)
   		:timestamp_(timestamp),
		processer_(processer),
		coroutine_(coroutine),
		interval_(interval)	{
	}

	Timestamp getTimestamp() { return timestamp_; }
	Processer* getProcesser() { return processer_; }
	Coroutine::Ptr getCoroutine() { return coroutine_; }
	uint64_t getInterval() { return interval_; };
private:
	Timestamp timestamp_;
	Processer* processer_;
	Coroutine::Ptr coroutine_;
	uint64_t interval_;
};

class Processer;

int createTimerFd();

class TimerManager {
friend class Scheduler;
public:
	typedef std::function<void ()> Callback;
	TimerManager() 
		:timer_fd_(createTimerFd()) {}
	~TimerManager() {
		::close(timer_fd_);
	}
	
	void addTimer(Timestamp when, Coroutine::Ptr coroutine, Processer* processer, uint64_t interval = 0);
private:
	ssize_t readTimerFd();
	void resetTimerFd(Timestamp when);
	void dealWithExpiredTimer();

	int timer_fd_;
	std::multimap<Timestamp, Timer::Ptr> timer_map_;
	Mutex mutex_;
};

}

#endif
