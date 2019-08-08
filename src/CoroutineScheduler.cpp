#include "CoroutineScheduler.h"
#include "Log.h"

namespace melon {

static __thread CoroutineScheduler* t_scheduleInThisThread = nullptr;

const int kPollTimeMs = 10000;

CoroutineScheduler::CoroutineScheduler()
	:started_(false),
	mutex_(),
	poller_(new PollPoller()) {
	if (t_scheduleInThisThread != nullptr) {
		LOG_FATAL << "create two Scheduler in one thread";
	} else {
		t_scheduleInThisThread = this;
	}
}


void CoroutineScheduler::run() {
	Coroutine::Ptr cur;

	Coroutine::Ptr poll_coroutine = std::make_shared<Coroutine>([&](){
						poller_->poll(kPollTimeMs);
					});

	while (started_) {
		//todo:线程安全
		{
			//没有协程时执行poll协程
			if (coroutines_.empty()) {
				LOG_DEBUG << "no coroutine";
				cur = poll_coroutine;
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

void CoroutineScheduler::schedule(Coroutine::Ptr coroutine) {
	coroutines_.push_back(coroutine);
}

void CoroutineScheduler::updateChannel(Channel* channel) {
	poller_->updateChannel(channel);
}

void CoroutineScheduler::start() {
	started_ = true;
}

void CoroutineScheduler::stop() {
	started_ = false;
}

}
