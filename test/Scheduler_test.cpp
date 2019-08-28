#include "Scheduler.h"
#include "GoStyleSyntax.h"
#include "Log.h"

using namespace melon;

void bar() {
	LOG_INFO << "start bar";
	LOG_INFO << "leave bar";
}

void foo() {
	LOG_INFO << "start foo";
	go bar;
}

int main() {
	Logger::setLogLevel(LogLevel::INFO);
	LoggerSingletion::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	go foo;
	SchedulerSingleton::getInstance()->start();
	return 0;
}

