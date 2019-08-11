#ifndef _MELON_SCHEDULER_THREAD_H_
#define _MELON_SCHEDULER_THREAD_H_

#include "CoroutineScheduler.h"
#include "Mutex.h"
#include "Condition.h"
#include "Noncopyable.h"
#include "Thread.h"

namespace melon {

class SchedulerThread :public Noncopyable {
public:
	typedef std::shared_ptr<SchedulerThread> Ptr;

	SchedulerThread();
	~SchedulerThread();

	CoroutineScheduler* startSchedule();

private:
	void threadFunc();
	Thread thread_;	
	CoroutineScheduler* scheduler_;
	Mutex mutex_;
	Condition cond_;
};

}

#endif
