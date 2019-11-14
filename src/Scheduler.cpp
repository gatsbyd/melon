#include "Processer.h"
#include "Log.h"
#include "TimerManager.h"
#include "Scheduler.h"

#include <assert.h>
#include <signal.h>

class IgnoreSigpipe {
public:
	IgnoreSigpipe() {
		::signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigpipe signalObj;

namespace melon {

Scheduler::Scheduler(size_t thread_number)
	:thread_num_(thread_number),
	main_processer_(this),
	timer_manager_(new TimerManager()),
	thread_(std::bind(&Scheduler::start, this)),
	cond(mutex_) {
	assert(thread_number > 0);
	assert(Processer::GetProcesserOfThisThread() == nullptr);

}

Scheduler::~Scheduler() {
	stop();
}

void Scheduler::start() {
	if (running_) {
		return;
	}
	for (size_t i = 0; i < thread_num_ - 1; ++i) {
		threads_.push_back(std::make_shared<ProcessThread>(this));
	}

	//work_processer
	work_processers_.push_back(&main_processer_);
	for (const ProcessThread::Ptr& thread : threads_) {
		work_processers_.push_back(thread->startProcess());
	}

	timer_thread_ = std::make_shared<ProcessThread>(this);
	timer_processer_ = timer_thread_->startProcess();
	timer_processer_->addTask([&]() {
						while (true) {
							timer_manager_->dealWithExpiredTimer();
						}
					}, "timer");
	{
		MutexGuard lock(mutex_);
		running_ = true;
	}
	cond.notify();
	main_processer_.run();
}

void Scheduler::startAsync() {
	if (running_) {
		return;
	}
	thread_.start();
	{
		MutexGuard lock(mutex_);
		while (!running_) {
			cond.wait();
		}
	}
}

void Scheduler::stop() {
	if (!running_) return;
	running_ = false;

	for (auto processer : work_processers_) {
		processer->stop();
	}
	for (auto thread : threads_) {
		thread->join();
	}
	timer_processer_->stop();
	timer_thread_->join();
}

void Scheduler::addTask(Coroutine::Func task, std::string name) {
	Processer* picked = pickOneProcesser();	//thread-save

	assert(picked != nullptr);
	picked->addTask(task, name);	//thread-save
}

Processer* Scheduler::pickOneProcesser() {
	MutexGuard lock(mutex_);
	static size_t index = 0;

	assert(index < work_processers_.size());
	Processer* picked = work_processers_[index++];
	index = index % work_processers_.size();

	return picked;
}

void Scheduler::runAt(Timestamp when, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	timer_manager_->addTimer(when, coroutine, processer); //threa-save
}

void Scheduler::runAfter(uint64_t micro_delay, Coroutine::Ptr coroutine) {
	Timestamp when = Timestamp::now() + micro_delay;
	runAt(when, coroutine);
}

void Scheduler::runEvery(uint64_t interval, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	Timestamp when = Timestamp::now() + interval * Timestamp::kMicrosecondsPerSecond;
	timer_manager_->addTimer(when, coroutine, processer, interval);
}


}
