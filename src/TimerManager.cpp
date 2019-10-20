#include "Processer.h"
#include "Log.h"
#include "TimerManager.h"

#include <assert.h>
#include <unistd.h>

namespace melon {

int createTimerFd() {
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

	if (timerfd < 0) {
		LOG_FATAL << "timerfd_create:" << strerror(errno);
	}

	return timerfd;
}

bool TimerManager::findFirstTimestamp(const Timestamp& now, Timestamp& timestamp) {
	for (const auto& pair : timer_map_) {
		if (now < pair.first) {
			timestamp = pair.first;
			return true;
		}
	}
	return false;
}

void TimerManager::addTimer(Timestamp when, Coroutine::Ptr coroutine, Processer* processer, uint64_t interval) {
	Timer::Ptr timer = std::make_shared<Timer>(when, processer, coroutine, interval);
	bool earliest_timer_changed = false;
	{
		MutexGuard lock(mutex_);
		auto it = timer_map_.begin();
		if (it == timer_map_.end() || when < it->first) {
			earliest_timer_changed = true;
		}
		timer_map_.insert({when, timer});

		if (earliest_timer_changed) {
			resetTimerFd(when);	
		}
	}
}

void TimerManager::resetTimerFd(Timestamp when) {
	struct itimerspec new_value;
	bzero(&new_value, sizeof(new_value));

	int64_t micro_seconds_diff = when.getMicroSecondsFromEpoch() - Timestamp::now().getMicroSecondsFromEpoch();
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(micro_seconds_diff / Timestamp::kMicrosecondsPerSecond);
	ts.tv_nsec = static_cast<long>(micro_seconds_diff % Timestamp::kMicrosecondsPerSecond * 1000);

	new_value.it_value = ts;
	if (timerfd_settime(timer_fd_, 0, &new_value, nullptr)) {
		LOG_ERROR << "timerfd_settime:" << strerror(errno);
	}
	LOG_DEBUG << "reset timerfd at " << when;
}

ssize_t TimerManager::readTimerFd() {
	uint64_t num_of_expirations;
	ssize_t n = ::read(timer_fd_, &num_of_expirations, sizeof(uint64_t));
	if (n != sizeof num_of_expirations) {
		LOG_ERROR << "read " << n << " bytes instead of 8";
	}
	return n;
}

void TimerManager::dealWithExpiredTimer() {
	readTimerFd();

	std::vector<std::pair<Timestamp, Timer::Ptr>> expired;
	auto it_not_less_now = timer_map_.lower_bound(Timestamp::now());
	std::copy(timer_map_.begin(), it_not_less_now, back_inserter(expired));
	timer_map_.erase(timer_map_.begin(), it_not_less_now);

	for (const std::pair<Timestamp, Timer::Ptr>& pair : expired) {
		assert(pair.second->getProcesser() != nullptr);
		pair.second->getProcesser()->addTask(pair.second->getCoroutine());

		if (pair.second->getInterval() > 0) {
			Timestamp new_timestamp = pair.first + pair.second->getInterval() * Timestamp::kMicrosecondsPerSecond;
			Timer::Ptr timer = std::make_shared<Timer>(new_timestamp, 
								pair.second->getProcesser(), 
								std::make_shared<Coroutine>(pair.second->getCoroutine()->getCallback()), 
								pair.second->getInterval());
			{
				MutexGuard lock(mutex_);
				timer_map_.insert({new_timestamp, timer});
			}
		}
	}
	
	Timestamp timestamp;
	if (findFirstTimestamp(Timestamp::now(), timestamp)) {
		resetTimerFd(timestamp);
	}
}

}
