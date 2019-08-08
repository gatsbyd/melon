#ifndef _MELON_CHANNEL_H_
#define _MELON_CHANNEL_H_

#include <functional>

namespace melon {

class CoroutineScheduler;

class Channel {
public:
	typedef std::function<void ()> EventCallback;
	Channel(int fd, CoroutineScheduler* sheduler);

	void setReadCallback(EventCallback read_cb);
	void setWriteCallback(EventCallback write_cb);
	void setRevents(int revents);
	void enableReading();
	void disableReading();
	void handleEvent();
	void setIndex(int index);
	int index();
	int fd();
	int events();
	
private:
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	const int fd_;
	int events_;
	int revents_;
	int index_;

	CoroutineScheduler* scheduler_;

	EventCallback read_cb_;
	EventCallback write_cb_;
};

}

#endif
