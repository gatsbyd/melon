#ifndef _MELON_SCHEDULER_H_
#define _MELON_SCHEDULER_H_

#include "Noncopyable.h"
#include "Coroutine.h"
#include "ProcesserThread.h"
#include "Processer.h"
#include "Timestamp.h"

#include <vector>

namespace melon {
	
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
	Processer main_processer_;
	std::vector<Processer*> processers_;
	std::vector<SchedulerThread::Ptr> threads_;


	//单独一个线程处理任务
	Processer* timer_processer_;
	SchedulerThread::Ptr timer_thread_;
	std::unique_ptr<TimerManager> timer_manager_;
};

}

#endif
