#include "Scheduler.h"
#include "Log.h"

using namespace melon;

void bar() {
	LOG_INFO << "start bar";
	LOG_INFO << "leave bar";
}

void foo() {
	LOG_INFO << "start foo";
}

int main() {
	Logger::setLogLevel(LogLevel::INFO);

	return 0;
}

