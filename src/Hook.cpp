#include "Coroutine.h"
#include "CoroutineScheduler.h"
#include "Hook.h"
#include "Log.h"

#include "assert.h"
#include <dlfcn.h>
#include <errno.h>
#include <memory>
#include <string.h>

namespace melon {

struct HookIniter {
	HookIniter() {
		accept_origin = (accept_func)::dlsym(RTLD_NEXT, "accept");
		read_origin = (read_func)::dlsym(RTLD_NEXT, "read");
		write_origin = (write_func)::dlsym(RTLD_NEXT, "write");
		sleep_origin = (sleep_func)::dlsym(RTLD_NEXT, "sleep");
	}
};

static HookIniter hook_initer;

}

extern "C" {

accept_func accept_origin = nullptr;
read_func read_origin = nullptr;
write_func write_origin = nullptr;
sleep_func sleep_origin = nullptr;

unsigned int sleep(unsigned int seconds) {
	melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
	if (!scheduler) {
		LOG_FATAL << "call hooked api in non io thread";
	}
	melon::Coroutine::Yield();
	
}

int accept(int fd, struct sockaddr *peer, socklen_t *addrlen) {
	ssize_t n;
retry:
	do {
		n = accept_origin(fd, peer, addrlen);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		scheduler->updateEvent(fd, melon::Poller::kReadEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent);
		
		goto retry;
	}

	return n;
}

ssize_t read(int fd, void *buf, size_t count) {
	LOG_DEBUG << "call hooked read";
	ssize_t n;
retry:
	do {
		n = read_origin(fd, buf, count);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		scheduler->updateEvent(fd, melon::Poller::kReadEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent);
		
		goto retry;
	}

	return n;
}

ssize_t write(int fd, const void *buf, size_t count) {
	LOG_DEBUG << "call hooked write";
	ssize_t n;
retry:
	do {
		n = write_origin(fd, buf, count);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		scheduler->updateEvent(fd, melon::Poller::kWriteEvent, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent);

		goto retry;
	}

	return n;
}

}



