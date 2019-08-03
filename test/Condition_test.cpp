#include "Condition.h"
#include "Log.h"
#include "Thread.h"

#include <memory>
#include <unistd.h>
#include <vector>


int main() {
	melon::Mutex mutex;
	melon::Condition cond(mutex);

	std::vector<std::shared_ptr<melon::Thread>> v;

	for (int i = 0; i < 10; ++i) {
		v.push_back(std::make_shared<melon::Thread>([&]() {
									melon::MutexGuard guard(mutex);
									cond.wait();
									LOG_DEBUG << "continue";
								}));
	}

	for (auto& i : v) {
		i->start();
	}

	sleep(5);
	cond.notifyAll();
	sleep(5);
	return 0;
}
