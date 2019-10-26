#include "Processer.h"
#include "Log.h"
#include "TimerManager.h"
#include "Scheduler.h"

#include <assert.h>

namespace melon {

Scheduler::Scheduler(size_t thread_number)
	:main_processer_(this),
	timer_manager_(new TimerManager()) {
	assert(thread_number > 0);
	assert(Processer::GetProcesserOfThisThread() == nullptr);

	for (size_t i = 0; i < thread_number - 1; ++i) {
		threads_.push_back(std::make_shared<SchedulerThread>(this));
	}

	//work_processer
	work_processers_.push_back(&main_processer_);
	for (const SchedulerThread::Ptr& thread : threads_) {
		work_processers_.push_back(thread->startSchedule());
	}

	timer_thread_ = std::make_shared<SchedulerThread>(this);
	timer_processer_ = timer_thread_->startSchedule();
}

Scheduler::~Scheduler() {
	LOG_INFO << "start destroy Scheduler";
	stop();
	LOG_INFO << "finish destroy Scheduler";
}

void Scheduler::start() {
	timer_processer_->addTask([&]() {
						while (true) {
							timer_manager_->dealWithExpiredTimer();
						}
					}, "timer");
	main_processer_.run();
	LOG_INFO << "leave Scheduler::start";
}

void Scheduler::stop() {
	if (stop_) return;
	stop_ = true;

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

void Scheduler::runEvery(uint64_t interval, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	Timestamp when = Timestamp::now() + interval * Timestamp::kMicrosecondsPerSecond;
	timer_manager_->addTimer(when, coroutine, processer, interval);
}


}
