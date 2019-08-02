#include <assert.h>
#include <atomic>
#include "Log.h"
#include <sys/syscall.h>
#include <Thread.h>
#include <unistd.h>

namespace melon {

std::atomic_int threadCount(0);
__thread pid_t t_tid = 0;

pid_t Thread::CurrentThreadTid() {
	if (t_tid == 0) {
		t_tid = ::syscall(SYS_gettid);
	}
	return t_tid;
}

Thread::Thread(Func cb, std::string name)
	: started_(false), joined_(false),
	   	cb_(std::move(cb)) {
	if (name.empty()) {
		int num = threadCount.fetch_add(1);
		char buf[30];
		snprintf(buf, sizeof buf, "Thread-%d", num);
		name_ = buf;
	} else {
		name_ = name;
	}
}

Thread::~Thread() {
	if (started_ && !joined_) {
		pthread_detach(tid_);
		LOG_DEBUG << "pthread_detach: " << Thread::CurrentThreadTid();
	}
}

void Thread::start() {
	assert(!started_);
	started_ = true;
	if (pthread_create(&tid_, nullptr, Thread::threadFuncInternal, this)) {
		started_ = false;	
		LOG_FATAL << "pthread_create failed";	
	}
}

void Thread::join() {
	assert(started_);
	assert(!joined_);
	joined_ = true;
	if (pthread_join(tid_, nullptr)) {
		joined_ = false;
		LOG_FATAL << "pthread_join failed";	
	}
}

const std::string& Thread::getName() const {
	return name_;
}

void* Thread::threadFuncInternal(void* arg) {
	Thread* thread = static_cast<Thread*>(arg);	
	Func cb;
	cb.swap(thread->cb_);

	//todo: exception
	cb();
	return 0;
}

}
