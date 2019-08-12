#include "SchedulerThread.h"


namespace melon {

SchedulerThread::SchedulerThread() 
	:thread_(std::bind(&SchedulerThread::threadFunc, this)),
	scheduler_(nullptr),
	mutex_(),
	cond_(mutex_) {

}

SchedulerThread::~SchedulerThread() {

}

CoroutineScheduler* SchedulerThread::startSchedule() {
	thread_.start();

	MutexGuard guard(mutex_);
	while (scheduler_ == nullptr) {
		cond_.wait();
	}
	scheduler_->start();
	return scheduler_;
}

void SchedulerThread::threadFunc() {
	CoroutineScheduler scheduler;
	scheduler_ = &scheduler;
	cond_.notify();

	scheduler.run();
}

}
