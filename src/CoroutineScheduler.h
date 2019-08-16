#ifndef _MELON_COROUTINE_SCHEDULER_H_
#define _MELON_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "Poller.h"
#include "TimerManager.h"

#include <list>

namespace melon {


class CoroutineScheduler : public Noncopyable {
public:
	typedef std::shared_ptr<CoroutineScheduler> Ptr;

	CoroutineScheduler();
	virtual ~CoroutineScheduler() {}

	virtual void run();
	void stop();
	void schedule(Coroutine::Ptr coroutine);
	void schedule(Coroutine::Func func, std::string name = "");
	void updateEvent(int fd, int events, Coroutine::Ptr coroutine = nullptr);
	void removeEvent(int fd);

	void scheduleAt(Timestamp when, Coroutine::Ptr coroutine);

	static CoroutineScheduler* GetSchedulerOfThisThread();

private:
	void wakeupPollCoroutine();
	ssize_t comsumeWakeEvent();

	bool started_;
	Mutex mutex_;
	PollPoller poller_;
	TimerManager timer_manager_;
	int event_fd_;
	std::list<Coroutine::Ptr> coroutines_;
};

}

#endif
