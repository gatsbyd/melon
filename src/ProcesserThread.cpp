#include "Processer.h"
#include "Log.h"
#include "ProcesserThread.h"

#include <assert.h>

namespace melon {

ProcessThread::ProcessThread(Scheduler* scheduler) 
	:thread_(std::bind(&ProcessThread::threadFunc, this)),
	scheduler_(scheduler),
	processer_(nullptr),
	mutex_(),
	cond_(mutex_) {

}

ProcessThread::~ProcessThread() {
}

Processer* ProcessThread::startProcess() {
	thread_.start();

	MutexGuard guard(mutex_);
	while (processer_ == nullptr) {
		cond_.wait();
	}
	assert(processer_ != nullptr);
	return processer_;
}

void ProcessThread::join() {
	thread_.join();
}

void ProcessThread::threadFunc() {
	Processer processer(scheduler_);

	{
		MutexGuard guard(mutex_);
		processer_ = &processer;
		cond_.notify();
	}

	processer.run();
}

}
