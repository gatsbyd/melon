#ifndef _MELON_CHANNEL_H_
#define _MELON_CHANNEL_H_

#include "Coroutine.h"

#include <functional>
#include <memory>

namespace melon {

class CoroutineScheduler;

class PollEvent {
public:
	typedef std::shared_ptr<PollEvent> Ptr;
	PollEvent(int fd, Coroutine::Ptr coroutine);

	int fd();
	int events();
	Coroutine::Ptr coroutine();
	void setReadEvent();
	void unSetReadEvent();
	void setWriteEvent();
	void unSetWriteEvent();
	
private:
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	const int fd_;
	int events_;
	Coroutine::Ptr coroutine_;
};

}

#endif
