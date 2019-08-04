#ifndef _MELON_COUNT_DOWN_LATCH_H_
#define _MELON_COUNT_DOWN_LATCH_H_

#include "Noncopyable.h"
#include "Condition.h"

namespace melon {

class CountDownLatch : public Noncopyable {
public:
	CountDownLatch(int count);

	void wait();
	void countDown();
private:
	int count_;
	Mutex mutex_;
	Condition cond_;
};

}

#endif
