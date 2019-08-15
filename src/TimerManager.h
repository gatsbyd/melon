#ifndef _MELON_TIMER_MANAGER_H_
#define _MELON_TIMER_MANAGER_H_

#include "Coroutine.h"
#include "Timestamp.h"

#include <map>
#include <memory>
#include <sys/timerfd.h>
#include <string.h>

namespace melon {

class Timer {
public:
	typedef std::shared_ptr<Timer> Ptr;
	Timer(Timestamp timestamp, Coroutine::Ptr coroutine, uint64_t interval);

	Timestamp getTimestamp() { return timestamp_; }
	Coroutine::Ptr getCoroutine() { return coroutine_; }
	uint64_t getInterval() { return interval_; };
private:
	Timestamp timestamp_;
	Coroutine::Ptr coroutine_;
	uint64_t interval_;
};

class CoroutineScheduler;

int createTimerFd();

class TimerManager {
friend class CoroutineScheduler;
public:
	TimerManager(CoroutineScheduler* scheduler) 
		:timer_fd_(createTimerFd()),
		scheduler_(scheduler) {}
	
	void addTimer(Timestamp when, Coroutine::Ptr coroutine, uint64_t interval = 0);
private:
	void readTimerFd();
	void resetTimerFd(Timestamp when);
	void dealWithExpiredTimer();

	int timer_fd_;
	CoroutineScheduler* scheduler_;
	std::multimap<Timestamp, Timer::Ptr> timer_map_;
};

}

#endif
