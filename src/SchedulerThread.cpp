#include "Log.h"
#include "SchedulerThread.h"

#include <assert.h>

namespace melon {

SchedulerThread::SchedulerThread() 
	:thread_(std::bind(&SchedulerThread::threadFunc, this)),
	scheduler_(nullptr),
	mutex_(),
	cond_(mutex_) {

}

SchedulerThread::~SchedulerThread() {
	//todo:
	
}

CoroutineScheduler* SchedulerThread::startSchedule() {
	thread_.start();

	MutexGuard guard(mutex_);
	while (scheduler_ == nullptr) {
		cond_.wait();
	}
	assert(scheduler_ != nullptr);
	return scheduler_;
}

void SchedulerThread::threadFunc() {
	CoroutineScheduler scheduler;

	{
		MutexGuard guard(mutex_);
		scheduler_ = &scheduler;
		cond_.notify();
	}

	scheduler.run();
}

}
