#include "Condition.h"

#include <errno.h>
#include <stdint.h>

namespace melon {

Condition::Condition(Mutex& mutex)
	:mutex_(mutex) {
	pthread_cond_init(&cond_, nullptr);
}

Condition::~Condition() {
	pthread_cond_destroy(&cond_);
}

void Condition::wait() {
	pthread_cond_wait(&cond_, mutex_.getMutex());
}

bool Condition::wait_seconds(time_t seconds) {
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME, &abstime);

	const int64_t kNanoSecondsPerSecond = 1000000000;
	int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

	abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
	abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);
	return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.getMutex(), &abstime);
}

void Condition::notify() {
	pthread_cond_signal(&cond_);
}

void Condition::notifyAll() {
	pthread_cond_broadcast(&cond_);
}

}
