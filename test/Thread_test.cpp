#include "Thread.h"
#include "Log.h"
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;
using namespace melon;

void func() {
	LOG_DEBUG << "in thread: " << Thread::CurrentThreadTid();
}

int main() {
	Thread t1(func, "2222");
	Thread t2(func);

	t1.start();
	t2.start();

	t1.join();	
	t2.join();
	LOG_DEBUG << "in thread: " << Thread::CurrentThreadTid();
	LOG_DEBUG << t1.getName();
	LOG_DEBUG << t2.getName();
	return 0;
}
