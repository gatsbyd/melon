#include "Poller.h"
#include "CoroutineScheduler.h"
#include "Log.h"

#include <error.h>
#include <assert.h>
#include <string.h>

namespace melon {

const int Poller::kNoneEvent = 0;
const int Poller::kReadEvent = POLL_IN | POLL_PRI;
const int Poller::kWriteEvent = POLL_OUT;

PollPoller::PollPoller(CoroutineScheduler* scheduler)
	:scheduler_(scheduler) {
}	

void PollPoller::updateEvent(int fd, int events, Coroutine::Ptr coroutine) {
	auto it = fd_to_index_.find(fd);
	if (it == fd_to_index_.end()) {
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = events;
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		fd_to_index_[fd] = pollfds_.size() - 1;
		fd_to_coroutine_[fd] = coroutine;
	} else {
		int index = it->second;
		pollfds_[index].events = events;
		pollfds_[index].revents = 0;
		fd_to_coroutine_[fd] = coroutine;
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
