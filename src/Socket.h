#ifndef _MELON_SOCKET_H_
#define _MELON_SOCKET_H_

#include <memory>

namespace melon {

class IpAddress;
class CoroutineScheduler;

class Socket {
public:
	typedef std::shared_ptr<Socket> Ptr;

	explicit Socket(int fd, CoroutineScheduler* scheduler) 
		:fd_(fd), scheduler_(scheduler) {}
	~Socket();

	void bind(const IpAddress& local);
	void listen();
	int accept(IpAddress& peer);

	int fd() const;

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);

	ssize_t read(void *buf, size_t count);
	ssize_t write(const void *buf, size_t count);
	void shutdownWrite();

	static void SetNonBlockAndCloseOnExec(int fd);
	static int CreateSocket();
private:
	int fd_;
	CoroutineScheduler* scheduler_;
};

}

#endif
