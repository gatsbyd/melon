#include "Poller.h"
#include "Processer.h"
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

PollPoller::PollPoller(Processer* processer)
	:is_polling(false),
	processer_(processer) {
}	

void PollPoller::updateEvent(int fd, int events, Coroutine::Ptr coroutine) {
	assert(coroutine != nullptr);
	auto it = fd_to_index_.find(fd);
	if (it == fd_to_index_.end()) {
		struct pollfd pfd;
		pfd.fd = fd;
		pfd.events = events;
		pfd.revents = kNoneEvent;
		pollfds_.push_back(pfd);
		fd_to_index_[fd] = pollfds_.size() - 1;
		fd_to_coroutine_[fd] = coroutine;

		std::string coroutine_name = coroutine->name();
		LOG_DEBUG << "register:<" << fd << ", " << eventToString(events) << ", " << coroutine_name  << ">";
	} else {
		int index = it->second;
		struct pollfd& pfd = pollfds_[index];
		
		LOG_DEBUG << "update:<" << fd << ", " << eventToString(pfd.events) << ", " << fd_to_coroutine_[fd]->name() << "> to " << "<" << fd << ", " << eventToString(events) << ", " <<  coroutine->name() << ">";

		pfd.events = events;
		pfd.revents = kNoneEvent;
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
	while (!processer_->stoped()) {
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
					--num;
					auto coroutine = fd_to_coroutine_[pollfd.fd];
					assert(coroutine != nullptr);
					LOG_DEBUG << "new event arrive:<" << pollfd.fd << ", " << eventToString(pollfd.revents) << ", " << coroutine->name() << ">";

					removeEvent(pollfd.fd);

					coroutine->setState(CoroutineState::RUNNABLE);
					processer_->addTask(coroutine);
				}	

				if (num == 0) {
					break;
				}
			}
		}
		Coroutine::SwapOut();
	}
}

}
