#include "CoroutineScheduler.h"
#include "Log.h"

#include <unistd.h>

using namespace melon;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	CoroutineScheduler scheduler;
	scheduler.schedule([](){
						LOG_DEBUG << "before sleep";
						sleep(5);
						LOG_DEBUG << "after sleep";
					});
	scheduler.run();
	return 0;
}
