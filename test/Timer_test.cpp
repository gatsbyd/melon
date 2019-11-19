#include "Scheduler.h"

#include <unistd.h>

using namespace melon;

int main() {
	Scheduler scheduler;
	scheduler.startAsync();

	int64_t timer_id = scheduler.runEvery(2 * Timestamp::kMicrosecondsPerSecond, std::make_shared<Coroutine>([](){
									printf("timeout\n");
								}));
	sleep(5);
	scheduler.cancel(timer_id);
	printf("cancel\n");

	getchar();
	return 0;
}
