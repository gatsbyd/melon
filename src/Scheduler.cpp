#include "Processer.h"
#include "Log.h"
#include "TimerManager.h"
#include "Scheduler.h"

#include <assert.h>

namespace melon {

Scheduler::Scheduler()
	:main_processer_(this),
	 timer_manager_(new TimerManager()){

	processers_.push_back(&main_processer_);
}

Scheduler::~Scheduler() {
	stop();
}

void Scheduler::start(size_t thread_number) {
	assert(thread_number > 0);

	for (size_t i = 0; i < thread_number - 1; ++i) {
		threads_.push_back(std::make_shared<SchedulerThread>(this));
		processers_.push_back(threads_.back()->startSchedule());
	}

	timer_thread_ = std::make_shared<SchedulerThread>(this);
	timer_processer_ = timer_thread_->startSchedule();

	timer_processer_->addTask([&]() {
						while (true) {
							timer_manager_->dealWithExpiredTimer();
						}
					}, "timer");
	assert(thread_number == processers_.size());
	main_processer_.run();
}

void Scheduler::stop() {
	if (stop_) return;
	stop_ = true;

	for (auto processer : processers_) {
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
	assert(index < processers_.size());
	Processer* picked = processers_[index];
	index = index % processers_.size();

	return picked;
}

void Scheduler::runAt(Timestamp when, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	timer_manager_->addTimer(when, coroutine, processer); //threa-save
}

}
