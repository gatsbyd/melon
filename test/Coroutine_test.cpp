#include "Coroutine.h"
#include "Log.h"

#include <vector>

using namespace melon;

static int sum = 0;

void test() {
	LOG_DEBUG << "in Coroutine(" << Coroutine::GetCid() << ")";
	Coroutine::SwapOut();
	sum++;
	LOG_DEBUG << "in Coroutine(" << Coroutine::GetCid() << ")";
	Coroutine::SwapOut();
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	const int sz = 1000;
	std::vector<Coroutine::Ptr> coroutines;
	for (int i = 0; i < sz; ++i) {
		coroutines.push_back(std::make_shared<Coroutine>(test));
	}

	for (int i = 0; i < sz; ++i) {
		coroutines[i]->swapIn();
		LOG_DEBUG << "back to main Coroutine(" << Coroutine::GetCid() << ")";
	}
	for (int i = 0; i < sz; ++i) {
		coroutines[i]->swapIn();
		LOG_DEBUG << "back to main Coroutine(" << Coroutine::GetCid() << ")";
	}

	LOG_DEBUG << "all coroutine terminated, " << "sum= " << sum;

	getchar();

	return 0;
}
