#ifndef _MELON_COROUTINE_SCHEDULER_H_
#define _MELON_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "Poller.h"

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
	void updateChannel(Channel* channel);

private:
	bool started_;
	Mutex mutex_;
	Poller::Ptr poller_;
	std::list<Coroutine::Ptr> coroutines_;
};

}

#endif
