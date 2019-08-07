#ifndef _MELON_POLLER_H_
#define _MELON_POLLER_H_

#include "Channel.h"
#include "Noncopyable.h"

#include <map>
#include <vector>
#include <poll.h>

namespace melon {

class Poller : public Noncopyable {
public:
	Poller();
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
