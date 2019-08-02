#include "gtest/gtest.h"
#include "Thread.h"
#include "Log.h"
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;
using namespace melon;

void func() {
	LOG_DEBUG << "in thread: " << Thread::CurrentThreadTid();
}

TEST(TestThread, BaseTest) {
	Thread t1(func);
	Thread t2(func);

	t1.start();
	t2.start();

	//t1.join();	
	//t2.join();
	//
	LOG_DEBUG << "in thread: " << Thread::CurrentThreadTid();
}
