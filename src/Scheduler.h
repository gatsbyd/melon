#ifndef _MELON_SCHEDULER_H_
#define _MELON_SCHEDULER_H_

#include "Coroutine.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "ProcesserThread.h"
#include "Processer.h"
#include "Singleton.h"
#include "Timestamp.h"

#include <vector>

namespace melon {
	
class TimerManager;

typedef Singleton<Scheduler> SchedulerSingleton;

class Scheduler : public Noncopyable {
friend class Singleton<Scheduler>;
public:
	typedef std::shared_ptr<Scheduler> Ptr;

	void start(size_t thread_number = 1);
	void stop();
	void addTask(Coroutine::Func task, std::string name = "");
	void runAt(Timestamp when, Coroutine::Ptr coroutine);

protected:
	Processer* pickOneProcesser();
private:
	Scheduler();
	~Scheduler();

	bool stop_ = false;
	Processer main_processer_;
	std::vector<Processer*> processers_;
	std::vector<SchedulerThread::Ptr> threads_;

	//单独一个线程处理任务
	Processer* timer_processer_;
	SchedulerThread::Ptr timer_thread_;
	std::unique_ptr<TimerManager> timer_manager_;

	Mutex mutex_;
};

}

#endif
