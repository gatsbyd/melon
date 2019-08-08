#include "Log.h"
#include "CoroutineScheduler.h"


using namespace melon;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	CoroutineScheduler scheduler;
	scheduler.start();
	scheduler.run();

	return 0;
}
