#ifndef _MELON_SOCKET_H_
#define _MELON_SOCKET_H_

#include <memory>

namespace melon {

class IpAddress;

class Socket {
public:
	typedef std::shared_ptr<Socket> Ptr;

	Socket(int fd) :fd_(fd) {}
	~Socket();

	void bind(const IpAddress& local);
	void listen();
	int accept(IpAddress& peer);

	int fd() const;

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);

	static void setNonBlockAndCloseOnExec(int fd);
	static int CreateSocket();
private:
	int fd_;
};

}

#endif
