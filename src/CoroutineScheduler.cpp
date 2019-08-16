#include "CoroutineScheduler.h"
#include "Hook.h"
#include "Log.h"

#include <errno.h>
#include <sys/eventfd.h>
#include <string.h>
#include <unistd.h>

namespace melon {

static __thread CoroutineScheduler* t_scheduleInThisThread = nullptr;

//todo:
const int kPollTimeMs = -10000;

static int createEventFd() {
	int event_fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (event_fd == -1) {
		LOG_FATAL << "create eventfd failed, errno=" << errno << " error str:" << strerror(errno);
	}
	return event_fd;
}

CoroutineScheduler::CoroutineScheduler()
	:started_(false),
	mutex_(),
	poller_(this),
	timer_manager_(this),
	event_fd_(createEventFd()) {
	
	if (t_scheduleInThisThread != nullptr) {
		LOG_FATAL << "create two Scheduler in one thread";
	} else {
		t_scheduleInThisThread = this;
	}

	//当有新事件到来时唤醒poll协程
	
	schedule([&](){
				while (true) {
					if (comsumeWakeEvent() < 0) {
						LOG_ERROR << "read eventfd:" << strerror(errno);
						break;
					}
				}
			}, "Wake");

	schedule([&](){
				while (true) {
					if (timer_manager_.readTimerFd() < 0) {
						LOG_ERROR << "read timerfd:" << strerror(errno);
						break;
					}
					timer_manager_.dealWithExpiredTimer();
				}
			}, "Timer");
}


void CoroutineScheduler::run() {
	started_ = true;
	Coroutine::Ptr cur;

	Coroutine::Ptr poll_coroutine = std::make_shared<Coroutine>(std::bind(&Poller::poll, &poller_, kPollTimeMs), "Poll");

	while (started_) {
		{
			MutexGuard guard(mutex_);
			//没有协程时执行poll协程
			if (coroutines_.empty()) {
				cur = poll_coroutine;
			} else {
				for (auto it = coroutines_.begin();
						it != coroutines_.end();
							++it) {
					//todo:条件
					//引入优先级的概念
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
	MutexGuard guard(mutex_);
	coroutines_.push_back(coroutine);

	if (poller_.isPoliing()) {
		wakeupPollCoroutine();
	}
}

void CoroutineScheduler::schedule(Coroutine::Func func, std::string name) {
	schedule(std::make_shared<Coroutine>(std::move(func), name));
}

void CoroutineScheduler::updateEvent(int fd, int events, Coroutine::Ptr coroutine) {
	poller_.updateEvent(fd, events, coroutine);
}
	
void CoroutineScheduler::removeEvent(int fd) {
	poller_.removeEvent(fd);
}

void CoroutineScheduler::stop() {
	started_ = false;
}

void CoroutineScheduler::wakeupPollCoroutine() {
	uint64_t buffer = 1;
	ssize_t n = ::write(event_fd_, &buffer, sizeof buffer);
	if (n != sizeof buffer) {
		LOG_ERROR << "wakeupPollCoroutine() size of the supplied buffer is not 8 bytes";
	}
}

ssize_t CoroutineScheduler::comsumeWakeEvent() {
	uint64_t buffer = 1;
	ssize_t n = ::read(event_fd_, &buffer, sizeof buffer);
	if (n != sizeof buffer) {
		LOG_ERROR << "comsumeWakeEvent() size of the data is not 8 bytes";
	}
	return n;
}

//一段时间后执行指定协程
void CoroutineScheduler::scheduleAt(Timestamp when, Coroutine::Ptr coroutine) {
	timer_manager_.addTimer(when, coroutine);
}

CoroutineScheduler* CoroutineScheduler::GetSchedulerOfThisThread() {
	return t_scheduleInThisThread;
}

}
