#include "Poller.h"
#include "Processer.h"
#include "Log.h"

#include <error.h>
#include <assert.h>
#include <string.h>
#include <sstream>

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
	:is_polling_(false),
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
	} else {
		size_t index = it->second;
		assert(index < pollfds_.size());
		struct pollfd& pfd = pollfds_[index];

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
		is_polling_ = true;
		int num = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
		is_polling_ = false;
		if (num == 0) {
		} else if (num < 0) {
			if (errno != EINTR) {
				LOG_ERROR << "poll error, errno: " << errno << ", error str:" << strerror(errno);
			}
		} else {
			std::vector<int> active_fds;
			for (const auto& pollfd : pollfds_) {
				if (pollfd.revents > 0) {
					--num;
					active_fds.push_back(pollfd.fd);
					if (num == 0) {
						break;
					}
				}
			}
			for (const auto& active_fd : active_fds) {
				auto coroutine = fd_to_coroutine_[active_fd];
				assert(coroutine != nullptr);

				removeEvent(active_fd);

				//todo:有四类事件：1.可读，2.可写，3.关闭，4.错误 需要处理
				coroutine->setState(CoroutineState::RUNNABLE);
				processer_->addTask(coroutine);
			}	
		}
		Coroutine::SwapOut();
	}
}

}
