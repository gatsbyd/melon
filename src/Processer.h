#ifndef _MELON_COROUTINE_SCHEDULER_H_
#define _MELON_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "Poller.h"

#include <list>

namespace melon {

class Scheduler;
class Processer : public Noncopyable {
public:
	typedef std::shared_ptr<Processer> Ptr;

	Processer(Scheduler* scheduler);
	virtual ~Processer() {}

	virtual void run();
	void stop();
	bool stoped() { return stop_; }
	size_t getLoad() { return load_; }
	Scheduler* getScheduler() { return scheduler_; }
	void addTask(Coroutine::Ptr coroutine);
	void addTask(Coroutine::Func func, std::string name = "");
	void updateEvent(int fd, int events, Coroutine::Ptr coroutine = nullptr);
	void removeEvent(int fd);

	static Processer*& GetProcesserOfThisThread();

private:
	void wakeupPollCoroutine();
	ssize_t comsumeWakeEvent();

	bool stop_ = false;
	size_t load_ = 0;
	Mutex mutex_;
	Scheduler* scheduler_;
	PollPoller poller_;
	int event_fd_;
	std::list<Coroutine::Ptr> coroutines_;
};

}

#endif
