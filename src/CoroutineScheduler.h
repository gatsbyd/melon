#ifndef _MELON_COROUTINE_SCHEDULER_H_
#define _MELON_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Noncopyable.h"

#include <list>

namespace melon {

class CoroutineScheduler : public Noncopyable {
public:
	CoroutineScheduler();
	virtual ~CoroutineScheduler() {}

	virtual void run();
	void start();
	void stop();
	void schedule(Coroutine::Ptr coroutine);

private:
	bool started_;
	Mutex mutex_;
	std::list<Coroutine::Ptr> coroutines_;
};

}

#endif
