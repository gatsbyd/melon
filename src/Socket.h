#ifndef _MELON_SOCKET_H_
#define _MELON_SOCKET_H_

#include "Noncopyable.h"

#include <memory>
#include <sys/uio.h>

namespace melon {

class IpAddress;
class Processer;

class Socket : public Noncopyable {
public:
	typedef std::shared_ptr<Socket> Ptr;

	explicit Socket(int fd) 
		:fd_(fd) {}

	void bind(const IpAddress& local);
	void listen();
	int accept(IpAddress& peer);
	int connect(IpAddress& server_addr);

	int fd() const;

	void setTcpNoDelay(bool on);
	void setReuseAddr(bool on);
	void setReusePort(bool on);
	void setKeepAlive(bool on);

	ssize_t read(void *buf, size_t count);
	ssize_t readv(const struct iovec* iov, int iovcnt);
	ssize_t write(const void *buf, size_t count);
	ssize_t writev(const struct iovec *iov, int iovcnt);
	void shutdownWrite();
	void close();

	void SetNonBlockAndCloseOnExec();
	static Socket::Ptr CreateTcp();
	static int GetSocketError(int sockfd);

private:
	int fd_;
};

}

#endif
