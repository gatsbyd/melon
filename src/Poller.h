#ifndef _MELON_POLLER_H_
#define _MELON_POLLER_H_

#include "Noncopyable.h"
#include "Channel.h"

#include <map>
#include <memory>
#include <vector>
#include <poll.h>

namespace melon {

class CoroutineScheduler;

class Poller : public Noncopyable {
public:
	typedef std::shared_ptr<Poller> Ptr;

	Poller() = default;
	virtual ~Poller() {};
	virtual void updateEvent(PollEvent::Ptr event) = 0;
	virtual void removeEvent(int fd) = 0;

	virtual void poll(int timeout) = 0;
};

class PollPoller : public Poller {
public:
	PollPoller(CoroutineScheduler* scheduler);

	void updateEvent(PollEvent::Ptr event) override;
	void removeEvent(int fd) override;

	void poll(int timeout) override;

private:
	std::vector<struct pollfd> pollfds_;
	std::map<int, Coroutine::Ptr> fd_to_coroutine_;
	std::map<int, int> fd_to_index_;
	CoroutineScheduler* scheduler_;
};

}

#endif
