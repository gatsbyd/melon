#include "Poller.h"
#include "Channel.h"
#include "Coroutine.h"
#include "Log.h"

#include <string.h>
#include <error.h>

namespace melon {

PollPoller::PollPoller() {

}
	
void PollPoller::updateChannel(Channel* channel) {
	if (channel->index() < 0) {
		struct pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short int>(channel->events());
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		int index = static_cast<int>(pollfds_.size()) - 1;
		channel->setIndex(index);
	} else {
		int index = channel->index();
		pollfds_[index].events = static_cast<short int>(channel->events());
		pollfds_[index].revents = 0;
	}	
}
	
void PollPoller::poll(int timeout) {
	while (true) {
		int num = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
		if (num == 0) {
			LOG_INFO << "PollPoller::poll nothing happened";
		} else if (num < 0) {
			if (errno != EINTR) {
				LOG_ERROR << "poll error, errno: " << errno << ", error str:" << strerror(errno);
			}
		} else {
			for (auto& pollfd : pollfds_) {
				if (pollfd.revents > 0) {
					--num;
					Channel* channel = fd_to_channel_[pollfd.fd];
					channel->setRevents(pollfd.revents);
					channel->handleEvent();
				}	

				if (num == 0) {
					break;
				}
			}
		}
		Coroutine::Yield();
	}
}

}
