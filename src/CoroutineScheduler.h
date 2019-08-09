#ifndef _MELON_COROUTINE_SCHEDULER_H_
#define _MELON_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Channel.h"
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
	void schedule(Coroutine::Func func);
	void updateChannel(Channel* channel);

	static CoroutineScheduler* GetSchedulerOfThisThread();

private:
	void wakeupPollCoroutine();
	void comsumeWakeEvent();

	bool started_;
	Mutex mutex_;
	Poller::Ptr poller_;
	int event_fd_;
	Channel event_channel_;
	std::list<Coroutine::Ptr> coroutines_;
};

}

#endif
