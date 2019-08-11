#include "Poller.h"
#include "Channel.h"
#include "Coroutine.h"
#include "CoroutineScheduler.h"
#include "Log.h"

#include <error.h>
#include <assert.h>
#include <string.h>

namespace melon {

PollPoller::PollPoller(CoroutineScheduler* scheduler)
	:scheduler_(scheduler) {

}	

void PollPoller::updateEvent(PollEvent::Ptr event) {
	auto it = fd_to_index_.find(event->fd());
	if (it == fd_to_index_.end()) {
		struct pollfd pfd;
		pfd.fd = event->fd();
		pfd.events = event->events();
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		fd_to_index_[event->fd()] = pollfds_.size() - 1;
		fd_to_coroutine_[event->fd()] = event->coroutine();
	} else {
		int index = it->second;
		pollfds_[index].events = event->events();
		pollfds_[index].revents = 0;
		fd_to_coroutine_[event->fd()] = event->coroutine();
	}
}
	
void PollPoller::removeEvent(int fd) {
	auto it = fd_to_index_.find(fd);
	if (it == fd_to_index_.end()) {
		return;
	}
	int index = it->second;
	fd_to_coroutine_.erase(fd);
	fd_to_index_.erase(fd);
	//todo:可以优化
	pollfds_.erase(pollfds_.begin() + index);
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
					scheduler_->schedule(fd_to_coroutine_[pollfd.fd]);
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
