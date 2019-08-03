#include "Condition.h"

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

void Condition::notify() {
	pthread_cond_signal(&cond_);
}

void Condition::notifyAll() {
	pthread_cond_broadcast(&cond_);
}

}
