#include "Coroutine.h"
#include "Log.h"

using namespace melon;

void test() {
	LOG_DEBUG << "in Coroutine(" << Coroutine::GetCid() << ")";
	Coroutine::Yield();
	LOG_DEBUG << "in Coroutine(" << Coroutine::GetCid() << ")";
	Coroutine::Yield();
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	Coroutine::Ptr c1 = std::make_shared<Coroutine>(test);

	c1->resume();
	LOG_DEBUG << "back to main Coroutine(" << Coroutine::GetCid() << ")";
	c1->resume();
	LOG_DEBUG << "back to main Coroutine(" << Coroutine::GetCid() << ")";

	return 0;
}
