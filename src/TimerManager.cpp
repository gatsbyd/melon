#include "Processer.h"
#include "Log.h"
#include "TimerManager.h"

#include <assert.h>
#include <unistd.h>

namespace melon {

std::atomic<int64_t> Timer::s_sequence_creator_;

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

int64_t TimerManager::addTimer(Timestamp when, Coroutine::Ptr coroutine, Processer* processer, uint64_t interval) {
	Timer::Ptr timer = std::make_shared<Timer>(when, processer, coroutine, interval);
	bool earliest_timer_changed = false;
	{
		MutexGuard lock(mutex_);
		auto it = timer_map_.begin();
		if (it == timer_map_.end() || when < it->first) {
			earliest_timer_changed = true;
		}
		timer_map_.insert({when, timer});
		sequence_2_timestamp_.insert({timer->getSequence(), when});

		if (earliest_timer_changed) {
			resetTimerFd(when);	
		}
	}
	return timer->getSequence();
}

void TimerManager::cancel(int64_t timer_id) {
	{
		MutexGuard lock(mutex_);
		auto it = sequence_2_timestamp_.find(timer_id);
		if (it != sequence_2_timestamp_.end()) {
			timer_map_.erase(it->second);
			sequence_2_timestamp_.erase(it);
		} else {
			cancel_set_.insert(timer_id);
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
	{
		MutexGuard lock(mutex_);
		auto it_not_less_now = timer_map_.lower_bound(Timestamp::now());
		std::copy(timer_map_.begin(), it_not_less_now, back_inserter(expired));
		timer_map_.erase(timer_map_.begin(), it_not_less_now);
		for (auto& pair : expired) {
			sequence_2_timestamp_.erase(pair.second->getSequence());
		}
	}

	for (const std::pair<Timestamp, Timer::Ptr>& pair : expired) {
		Timer::Ptr old_timer = pair.second;
		{
			MutexGuard lock(mutex_);
			if (cancel_set_.find(old_timer->getSequence()) != cancel_set_.end()) {
				continue;
			}
		}

		assert(old_timer->getProcesser() != nullptr);
		old_timer->getProcesser()->addTask(old_timer->getCoroutine());

		if (old_timer->getInterval() > 0) {
			Timestamp new_timestamp = Timestamp::now() + old_timer->getInterval();
			old_timer->setTimestamp(new_timestamp);
			old_timer->setCoroutine(std::make_shared<Coroutine>(old_timer->getCoroutine()->getCallback()));
			{
				MutexGuard lock(mutex_);
				timer_map_.insert({new_timestamp, old_timer});
				sequence_2_timestamp_[old_timer->getSequence()] = new_timestamp;
			}
		}
	}
	{
		MutexGuard lock(mutex_);
		cancel_set_.clear();
	}
	
	Timestamp timestamp;
	if (findFirstTimestamp(Timestamp::now(), timestamp)) {
		resetTimerFd(timestamp);
	}
}

}
