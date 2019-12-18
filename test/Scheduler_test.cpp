#include "Scheduler.h"
#include "Log.h"

using namespace melon;

Scheduler* g_scheduler;

void foo() {
	std::cout << "in foo()" << std::endl;
	g_scheduler->stop();
	std::cout << "leave foo" << std::endl;
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler(3);
	g_scheduler = &scheduler;
	scheduler.startAsync();

	scheduler.addTask(foo);
	
	scheduler.wait();
	return 0;
}

