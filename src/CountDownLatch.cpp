#include "CountDownLatch.h"

namespace melon {

CountDownLatch::CountDownLatch(int count) 
	:count_(count),
	mutex_(),
	cond_(mutex_) {
	
}

void CountDownLatch::wait() {
	MutexGuard guard(mutex_);

	while (count_ > 0) {
		cond_.wait();
	}
}

void CountDownLatch::countDown() {
	MutexGuard guard(mutex_);

	count_--;
	if (count_ == 0) {
		cond_.notify();
	}
}


}
