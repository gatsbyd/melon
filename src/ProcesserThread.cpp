#include "Processer.h"
#include "Log.h"
#include "ProcesserThread.h"

#include <assert.h>

namespace melon {

SchedulerThread::SchedulerThread(Scheduler* scheduler) 
	:thread_(std::bind(&SchedulerThread::threadFunc, this)),
	scheduler_(scheduler),
	processer_(nullptr),
	mutex_(),
	cond_(mutex_) {

}

SchedulerThread::~SchedulerThread() {
}

Processer* SchedulerThread::startSchedule() {
	thread_.start();

	MutexGuard guard(mutex_);
	while (processer_ == nullptr) {
		cond_.wait();
	}
	assert(processer_ != nullptr);
	return processer_;
}

void SchedulerThread::join() {
	thread_.join();
}

void SchedulerThread::threadFunc() {
	Processer processer(scheduler_);

	{
		MutexGuard guard(mutex_);
		processer_ = &processer;
		cond_.notify();
	}

	processer.run();
}

}
