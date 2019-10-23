#include "Buffer.h"
#include "TcpConnection.h"

namespace melon {

TcpConnection::TcpConnection(Socket::Ptr socket, IpAddress peer) 
		:conn_socket_(socket),
		peer_addr_(peer) {
}

ssize_t TcpConnection::read(void* buf, size_t count) {
	return conn_socket_->read(buf, count);
}

ssize_t TcpConnection::readn(void* buf, size_t count) {
	size_t readn = 0;
	size_t left = count;
	while (left > 0) {
		ssize_t n = read(static_cast<char*>(buf) + readn, left);
		if (n <= 0) {
			return readn;
		}
		readn += n;
		left -= n;
	}
	return count;
}

ssize_t TcpConnection::read(Buffer* buf) {
	return buf->readSocket(conn_socket_);
}

ssize_t TcpConnection::write(const void* buf, size_t count) {
	return conn_socket_->write(buf, count);
}

ssize_t TcpConnection::writen(const void* buf, size_t count) {
	size_t writen = 0;
	size_t left = count;
	while (left > 0) {
		ssize_t n = write(static_cast<const char*>(buf) + writen, left);
		if (n <= 0) {
			return writen;
		}
		writen += n;
		left -= n;
	}
	return count;
}

void TcpConnection::shutdown() {
	conn_socket_->shutdownWrite();
}

ssize_t TcpConnection::write(Buffer* buf) {
	ssize_t n = writen(buf->peek(), buf->readableBytes());
	if (n > 0) {
		buf->retrieve(n);
	}
	return n;
}

ssize_t TcpConnection::write(const std::string& message) {
	return writen(message.data(), message.size());
}

}
