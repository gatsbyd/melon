#ifndef _MELON_SCHEDULER_THREAD_H_
#define _MELON_SCHEDULER_THREAD_H_

#include "Mutex.h"
#include "Condition.h"
#include "Noncopyable.h"
#include "Thread.h"

#include <memory>

namespace melon {

class Scheduler;
class Processer;

class SchedulerThread :public Noncopyable {
public:
	typedef std::shared_ptr<SchedulerThread> Ptr;

	SchedulerThread(Scheduler* scheduler);
	~SchedulerThread();

	Processer* startSchedule();
	void join();

private:
	void threadFunc();
	Thread thread_;	
	Scheduler* scheduler_;
	Processer* processer_;
	Mutex mutex_;
	Condition cond_;
};

}


#endif
