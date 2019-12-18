#ifndef _MELON_TCP_CONNECTION_H_
#define _MELON_TCP_CONNECTION_H_

#include "Address.h"
#include "Buffer.h"
#include "Socket.h"

namespace melon {

class TcpConnection {
public:
	typedef std::shared_ptr<TcpConnection> Ptr;
	explicit TcpConnection(Socket::Ptr socket, IpAddress peer);

	ssize_t read(void* buf, size_t count);
	ssize_t readn(void* buf, size_t count);
	ssize_t read(Buffer::Ptr);
	ssize_t write(const void* buf, size_t count);
	ssize_t writen(const void* buf, size_t count);
	ssize_t write(Buffer::Ptr);
	ssize_t write(const std::string& message);
	void shutdown();
	void readUntilZero();
	void close();
	void setTcpNoDelay(bool on);

	const IpAddress& peerAddr() const { return peer_addr_; }
private:
	Socket::Ptr conn_socket_;
	IpAddress peer_addr_;
};

}

#endif
