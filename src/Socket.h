#ifndef _MELON_SOCKET_H_
#define _MELON_SOCKET_H_



namespace melon {

class IpAddress;

class Socket {
public:
	Socket(int fd) :fd_(fd) {}

	void bind(const IpAddress& local);
	void listen();
	int accept(IpAddress& peer);

	int fd() const;

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);

	void setNonBlockAndCloseOnExec();
private:
	int fd_;
};

}

#endif
