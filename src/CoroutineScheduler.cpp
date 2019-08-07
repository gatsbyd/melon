#include "CoroutineScheduler.h"
#include "Log.h"

namespace melon {

static __thread CoroutineScheduler* t_scheduleInThisThread = nullptr;

CoroutineScheduler::CoroutineScheduler() {
	if (t_scheduleInThisThread != nullptr) {
		LOG_FATAL << "create two Scheduler in one thread";
	} else {
		t_scheduleInThisThread = this;
	}
}


void CoroutineScheduler::run() {
	
	Coroutine::Ptr cur;
	while (started_) {
		//todo:锁优化
		MutexGuard guard(mutex_);
		{
				if (coroutines_.empty()) {
					//todo:poll coroutine
				} else {
					for (auto it = coroutines_.begin();
								it != coroutines_.end();
								++it) {
						//todo:条件
						cur = *it;
						coroutines_.erase(it);
						break;
					}
				}

		}
		cur->resume();
	}
}

void CoroutineScheduler::start() {
	started_ = true;
}

void CoroutineScheduler::stop() {
	started_ = false;
}

void CoroutineScheduler::schedule(Coroutine::Ptr coroutine) {
	MutexGuard guard(mutex_);
	coroutines_.push_back(coroutine);
}

}
