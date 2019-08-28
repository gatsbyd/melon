#include "Scheduler.h"
#include "Log.h"

using namespace melon;

void bar() {
	LOG_INFO << "start bar";
	LOG_INFO << "leave bar";
}

void foo(melon::Scheduler* scheduler) {
	LOG_INFO << "start foo";
	scheduler->addTask(bar, "bar");
}

int main() {
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	melon::Scheduler scheduler;
	scheduler.addTask(std::bind(foo, &scheduler), "foo");
	scheduler.start();

	return 0;
}

