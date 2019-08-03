#include "Mutex.h"

namespace melon {

Mutex::Mutex() {
	pthread_mutex_init(&mutex_, nullptr);
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&mutex_);
}

void Mutex::lock() {
	pthread_mutex_lock(&mutex_);
}

void Mutex::unlock() {
	pthread_mutex_unlock(&mutex_);
}

pthread_mutex_t* Mutex::getMutex() {
	return &mutex_;
}

MutexGuard::MutexGuard(Mutex& mutex)
	:mutex_(mutex) {
	mutex_.lock();
}
MutexGuard::~MutexGuard() {
	mutex_.unlock();
}

}
