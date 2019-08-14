#include "Poller.h"
#include "CoroutineScheduler.h"
#include "Log.h"

#include <error.h>
#include <assert.h>
#include <string.h>

namespace melon {

const int Poller::kNoneEvent = 0;
const int Poller::kReadEvent = POLLIN | POLLPRI;
const int Poller::kWriteEvent = POLLOUT;
	
std::string Poller::eventToString(int event) {
  std::ostringstream oss;
  if (event & POLLIN)
    oss << "IN ";
  if (event & POLLPRI)
    oss << "PRI ";
  if (event & POLLOUT)
    oss << "OUT ";
  if (event & POLLHUP)
    oss << "HUP ";
  if (event & POLLRDHUP)
    oss << "RDHUP ";
  if (event & POLLERR)
    oss << "ERR ";
  if (event & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

PollPoller::PollPoller(CoroutineScheduler* scheduler)
	:is_polling(false),
	scheduler_(scheduler) {
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

		std::string coroutine_name = coroutine == nullptr ? "null" : coroutine->name();
		LOG_DEBUG << "register:<" << fd << ", " << eventToString(events) << ", " << coroutine_name  << ">";
	} else {
		int index = it->second;
		struct pollfd& pfd = pollfds_[index];
		
		std::string old_coroutine_name = fd_to_coroutine_[fd] == nullptr ? "null" : fd_to_coroutine_[fd]->name();
		std::string coroutine_name = coroutine == nullptr ? "null" : coroutine->name();
		LOG_DEBUG << "update:<" << fd << ", " << eventToString(pfd.events) << ", " << old_coroutine_name << "> to " << "<" << fd << ", " << eventToString(events) << ", " <<  coroutine_name << ">";

		pfd.events = events;
		pfd.revents = 0;
		fd_to_coroutine_[fd] = coroutine;
	}
}
	
void PollPoller::removeEvent(int fd) {
	auto it = fd_to_index_.find(fd);
	if (it == fd_to_index_.end()) {
		return;
	}
	LOG_DEBUG << "remove fd " << fd << " from poller";
	size_t index = it->second;

	fd_to_index_.erase(fd);
	fd_to_coroutine_.erase(fd);
	assert(index < pollfds_.size());
	if  (index == pollfds_.size() - 1) {
		pollfds_.pop_back();
	} else {
		int fd_at_end = pollfds_.back().fd;
		std::iter_swap(pollfds_.begin() + index, pollfds_.end() - 1);
		fd_to_index_[fd_at_end] = index;
		pollfds_.pop_back();
	}
}

void PollPoller::poll(int timeout) {
	while (true) {
		is_polling = true;
		int num = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
		is_polling = false;
		if (num == 0) {
			LOG_INFO << "PollPoller::poll nothing happened";
		} else if (num < 0) {
			if (errno != EINTR) {
				LOG_ERROR << "poll error, errno: " << errno << ", error str:" << strerror(errno);
			}
		} else {
			for (auto& pollfd : pollfds_) {
				if (pollfd.revents > 0) {
					assert(fd_to_coroutine_[pollfd.fd] != nullptr);
					--num;
					LOG_DEBUG << "new event arrive:<" << pollfd.fd << ", " << eventToString(pollfd.revents) << ", " << fd_to_coroutine_[pollfd.fd]->name() << ">";

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
