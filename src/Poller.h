#ifndef _MELON_POLLER_H_
#define _MELON_POLLER_H_

#include "Noncopyable.h"

#include <map>
#include <memory>
#include <vector>
#include <poll.h>

namespace melon {

class Channel;

class Poller : public Noncopyable {
public:
	typedef std::shared_ptr<Poller> Ptr;

	Poller() = default;
	virtual ~Poller() {};
	virtual void updateChannel(Channel* channel) = 0;

	virtual void poll(int timeout) = 0;
};

class PollPoller : public Poller {
public:
	PollPoller();

	void updateChannel(Channel* channel) override;

	void poll(int timeout) override;

private:
	std::vector<struct pollfd> pollfds_;
	std::map<int, Channel*> fd_to_channel_;
};

}

#endif
