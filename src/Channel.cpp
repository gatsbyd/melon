#include "Channel.h"
#include "CoroutineScheduler.h"
#include "Log.h"

#include <poll.h>

namespace melon {
	
const int PollEvent::kNoneEvent = 0;
const int PollEvent::kReadEvent = POLLIN | POLLPRI;
const int PollEvent::kWriteEvent = POLLOUT;

PollEvent::PollEvent(int fd,  Coroutine::Ptr coroutine)
	:fd_(fd),
	events_(kNoneEvent),
	coroutine_(coroutine) {

}

void PollEvent::setReadEvent() {
	events_ |= kReadEvent;
}

void PollEvent::unSetReadEvent() {
	events_ &= ~kReadEvent;
}

void PollEvent::setWriteEvent() {
	events_ |= kWriteEvent;
}

void PollEvent::unSetWriteEvent() {
	events_ &= ~kWriteEvent;
}

int PollEvent::fd() {
	return fd_;
}

int PollEvent::events() {
	return events_;
}

Coroutine::Ptr PollEvent::coroutine() {
	return coroutine_;
}

}
