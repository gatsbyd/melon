#include "CoroutineScheduler.h"
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
	event_fd_(createEventFd()) {
	
	if (t_scheduleInThisThread != nullptr) {
		LOG_FATAL << "create two Scheduler in one thread";
	} else {
		t_scheduleInThisThread = this;
	}
}


void CoroutineScheduler::run() {
	Coroutine::Ptr cur;

	Coroutine::Ptr poll_coroutine = std::make_shared<Coroutine>([&](){
						poller_.poll(kPollTimeMs);
					});

	while (started_) {
		//todo:线程安全
		{
			//没有协程时执行poll协程
			if (coroutines_.empty()) {
				LOG_DEBUG << "execute poll coroutine";
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
	bool need_notify = false;
	if (coroutines_.empty()) {
		need_notify = true;
	}
	coroutines_.push_back(coroutine);

	if (need_notify) {
		wakeupPollCoroutine();
	}
}

void CoroutineScheduler::schedule(Coroutine::Func func) {
	coroutines_.push_back(std::make_shared<Coroutine>(std::move(func)));
}

void CoroutineScheduler::updateEvent(PollEvent::Ptr event) {
	poller_.updateEvent(event);
}
	
void CoroutineScheduler::removeEvent(int fd) {
	poller_.removeEvent(fd);
}

void CoroutineScheduler::start() {
	started_ = true;
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

void CoroutineScheduler::comsumeWakeEvent() {
	uint64_t buffer = 1;
	ssize_t n = ::read(event_fd_, &buffer, sizeof buffer);
	if (n != sizeof buffer) {
		LOG_ERROR << "comsumeWakeEvent() size of the data is not 8 bytes";
	}
}

CoroutineScheduler* CoroutineScheduler::GetSchedulerOfThisThread() {
	return t_scheduleInThisThread;
}

}
