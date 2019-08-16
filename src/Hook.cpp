#include "Coroutine.h"
#include "CoroutineScheduler.h"
#include "Timestamp.h"
#include "Hook.h"
#include "Log.h"

#include "assert.h"
#include <dlfcn.h>
#include <errno.h>
#include <memory>
#include <string.h>

namespace melon {

#define DLSYM(name) \
		name ## _origin = (name ## _func)::dlsym(RTLD_NEXT, #name);

struct HookIniter {
	HookIniter() {
		DLSYM(sleep);
		DLSYM(accept);
		DLSYM(accept4);
		DLSYM(read);
		DLSYM(readv);
		DLSYM(recv);
		DLSYM(recvfrom);
		DLSYM(recvmsg);
		DLSYM(write);
		DLSYM(writev);
		DLSYM(send);
		DLSYM(sendto);
		DLSYM(sendmsg);
	}
};

static HookIniter hook_initer;

}

template<typename OriginFun, typename... Args>
static ssize_t ioHook(int fd, OriginFun origin_func, int event, Args&&... args) {
	ssize_t n;
retry:
	do {
		n = origin_func(fd, std::forward<Args>(args)...);
	} while (n == -1 && errno == EINTR);

	//todo:记得去掉EINVAL
	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINVAL)) {
		melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
		if (!scheduler) {
			LOG_FATAL << "call hooked api in non io thread";
		}

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		scheduler->updateEvent(fd, event, melon::Coroutine::GetCurrentCoroutine());
		melon::Coroutine::Yield();
		//清除事件
		scheduler->updateEvent(fd, melon::Poller::kNoneEvent);
		
		goto retry;
	}

	return n;
}

extern "C" {
#define HOOK_INIT(name) \
		name ## _func name ## _origin = nullptr;

HOOK_INIT(sleep)
HOOK_INIT(accept)
HOOK_INIT(accept4)
HOOK_INIT(read)
HOOK_INIT(readv)
HOOK_INIT(recv)
HOOK_INIT(recvfrom)
HOOK_INIT(recvmsg)
HOOK_INIT(write)
HOOK_INIT(writev)
HOOK_INIT(send)
HOOK_INIT(sendto)
HOOK_INIT(sendmsg)



unsigned int sleep(unsigned int seconds) {
	melon::CoroutineScheduler* scheduler = melon::CoroutineScheduler::GetSchedulerOfThisThread();
	if (!scheduler) {
		LOG_FATAL << "call hooked api in non io thread";
	}

	scheduler->scheduleAt(melon::Timestamp::now() + seconds * melon::Timestamp::kMicrosecondsPerSecond, melon::Coroutine::GetCurrentCoroutine());
	melon::Coroutine::Yield();
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return ioHook(sockfd, accept_origin, melon::Poller::kReadEvent, addr, addrlen);
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
	return ioHook(sockfd, accept4_origin, melon::Poller::kReadEvent, addr, addrlen, flags);
}

ssize_t read(int fd, void *buf, size_t count) {
	return ioHook(fd, read_origin, melon::Poller::kReadEvent, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, readv_origin, melon::Poller::kReadEvent, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return ioHook(sockfd, recv_origin, melon::Poller::kReadEvent, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, 
				struct sockaddr *src_addr, socklen_t *addrlen) {
	return ioHook(sockfd, recvfrom_origin, melon::Poller::kReadEvent, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	return ioHook(sockfd, recvmsg, melon::Poller::kReadEvent, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
	return ioHook(fd, write_origin, melon::Poller::kWriteEvent, buf, count);
}


ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, writev_origin, melon::Poller::kWriteEvent, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	return ioHook(sockfd, send_origin, melon::Poller::kWriteEvent, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, 
		const struct sockaddr *dest_addr, socklen_t addrlen) {
	return ioHook(sockfd, sendto_origin, melon::Poller::kWriteEvent, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	return ioHook(sockfd, sendmsg_origin, melon::Poller::kWriteEvent, msg, flags);	
}

}



