#ifndef _MELON_TCP_CONNECTION_H_
#define _MELON_TCP_CONNECTION_H_

#include "Socket.h"

namespace melon {

class TcpConnection {
public:
	typedef std::shared_ptr<TcpConnection> Ptr;
	explicit TcpConnection(Socket::Ptr socket) :socket_(socket) {}

	ssize_t read(void* buf, size_t count);
	ssize_t readFixSize(void* buf, size_t count);
	ssize_t write(const void* buf, size_t count);
	ssize_t writeFixSize(const void* buf, size_t count);
	void shutdown();

private:
	Socket::Ptr socket_;
};

}

#endif
