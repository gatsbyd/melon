#ifndef _MELON_SCHEDULER_H_
#define _MELON_SCHEDULER_H_

#include "Noncopyable.h"
#include "Coroutine.h"
#include "ProcesserThread.h"
#include "Timestamp.h"

#include <vector>

namespace melon {
	
class Processer;
class TimerManager;

class Scheduler : public Noncopyable {
public:
	typedef std::shared_ptr<Scheduler> Ptr;
	Scheduler();
	~Scheduler();

	void start(size_t thread_number = 1);
	void stop();
	void addTask(Coroutine::Func task, std::string name = "");
	void runAt(Timestamp when, Coroutine::Ptr coroutine);

protected:
	virtual Processer* pickOneProcesser();
private:
	bool stop_ = false;
	std::vector<Processer*> processers_;
	std::vector<SchedulerThread::Ptr> threads_;

	Processer* timer_processer_;
	SchedulerThread::Ptr timer_thread_;
	std::unique_ptr<TimerManager> timer_manager_;
};

}

#endif
