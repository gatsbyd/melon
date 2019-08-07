#include "Channel.h"
#include "Log.h"

#include <poll.h>

namespace melon {
	
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(int fd, CoroutineScheduler* scheduler) 
	:fd_(fd),
	events_(kNoneEvent),
	revents_(kNoneEvent),
	scheduler_(scheduler) {

}

void Channel::enableReading() {
	events_ |= kReadEvent;
	scheduler_->updateChannel(this);
}

void Channel::disableReading() {
	events_ &= ~kReadEvent;
	scheduler_->updateChannel(this);
}

void Channel::setReadCallback(EventCallback read_cb) {
	read_cb_ = std::move(read_cb);
}

void Channel::setWriteCallback(EventCallback write_cb) {
	write_cb_ = std::move(write_cb);
}

void Channel::handleEvent() {
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (read_cb_) {
			read_cb_();
		}
	} 
	if (revents_ & (POLLOUT)) { 
		if (write_cb_) {
			write_cb_();
		}
	}
}

void Channel::setRevents(int revents) {
	revents_ = revents;
}

void Channel::setIndex(int index) {
	index_ = index;
}

int Channel::index() {
	return index_;
}

int Channel::fd() {
	return fd_;
}

int Channel::events() {
	return events_;
}

}
