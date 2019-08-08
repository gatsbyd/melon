#include "Log.h"
#include "Channel.h"
#include "CoroutineScheduler.h"

#include <sys/timerfd.h>
#include <strings.h>
#include <unistd.h>

using namespace melon;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	CoroutineScheduler scheduler;

	int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
	Channel timefdChannel(timerfd, &scheduler);
	timefdChannel.setReadCallback([&](){
						LOG_DEBUG << "timeout!";
						uint64_t expired;
						read(timerfd, &expired, sizeof expired);

						scheduler.schedule([](){
								LOG_DEBUG << "run new coroutine " << Coroutine::GetCid();
							});
					});
	timefdChannel.enableReading();
	struct itimerspec timespec;
	bzero(&timespec, sizeof timespec);
	timespec.it_value.tv_sec = 3;
	::timerfd_settime(timerfd, 0, &timespec, NULL);

	scheduler.start();
	scheduler.run();

	::close(timerfd);
	return 0;
}
