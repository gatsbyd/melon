#ifndef _MELON_CHANNEL_H_
#define _MELON_CHANNEL_H_

#include "CoroutineScheduler.h"

namespace melon {

class Channel {
public:
	typedef std::function<void ()> EventCallback;
	Channel(int fd, CoroutineScheduler* sheduler);

	void setReadCallback(EventCallback read_cb);
	void setWriteCallback(EventCallback write_cb);
	void enableReading();
	void disableReading();
	void handleEvent();
	
private:
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	const int fd_;
	int events_;
	int revents_;

	CoroutineScheduler* scheduler_;

	EventCallback read_cb_;
	EventCallback write_cb_;
};

}

#endif
