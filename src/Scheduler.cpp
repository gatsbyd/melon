#include "Processer.h"
#include "Log.h"
#include "Hook.h"
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
	cond_(mutex_),
	quit_cond_(mutex_),
	join_thread_(std::bind(&Scheduler::joinThread, this)) {
	assert(thread_number > 0);
	assert(Processer::GetProcesserOfThisThread() == nullptr);

	//main_processer
	work_processers_.push_back(&main_processer_);
}

Scheduler::~Scheduler() {
	stop();
}

void Scheduler::start() {
	if (running_) {
		return;
	}
	//work_thread
	for (size_t i = 0; i < thread_num_ - 1; ++i) {
		threads_.push_back(std::make_shared<ProcessThread>(this));
	}

	//work_processer
	for (const ProcessThread::Ptr& thread : threads_) {
		work_processers_.push_back(thread->startProcess());
	}

	//timer_thread
	timer_thread_ = std::make_shared<ProcessThread>(this);
	//timer_processer
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
	cond_.notify();
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
			cond_.wait();
		}
	}
}

void Scheduler::wait() {
	quit_cond_.wait();
}

void Scheduler::stop() {
	if (!running_) 
		return;
	running_ = false;
	
	//main_processer
	main_processer_.stop();

	//work_processer
	for (auto processer : work_processers_) {
		processer->stop();
	}

	//timer_processer
	timer_processer_->stop();

	//如果stop在scheduler线程中调用,在新建的线程中join
	if (melon::isHookEnabled()) {
		join_thread_.start();
	} else {
		joinThread();
	}
}

void Scheduler::joinThread() {
	if (thread_.isStarted()) {
		thread_.join();
	}

	for (auto thread : threads_) {
		thread->join();
	}
	timer_thread_->join();
	quit_cond_.notify();
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

int64_t Scheduler::runAt(Timestamp when, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	return timer_manager_->addTimer(when, coroutine, processer); //threa-save
}

int64_t Scheduler::runAfter(uint64_t micro_delay, Coroutine::Ptr coroutine) {
	Timestamp when = Timestamp::now() + micro_delay;
	return runAt(when, coroutine);
}

int64_t Scheduler::runEvery(uint64_t micro_interval, Coroutine::Ptr coroutine) {
	Processer* processer = Processer::GetProcesserOfThisThread();
	if (processer == nullptr) {
		processer = pickOneProcesser();
	}
	Timestamp when = Timestamp::now() + micro_interval;
	return timer_manager_->addTimer(when, coroutine, processer, micro_interval);
}


void Scheduler::cancel(int64_t timer_id) {
	timer_manager_->cancel(timer_id);
}


}
