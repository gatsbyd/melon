#include "Socket.h"
#include "Address.h"
#include "Log.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>


namespace melon {

void Socket::bind(const IpAddress& local) {
	if (::bind(fd_, local.getSockAddr(), sizeof(struct sockaddr_in)) < 0) {
		LOG_FATAL << "bind: " << strerror(errno);
	}
}

void Socket::listen() {
	if (::listen(fd_, SOMAXCONN) < 0) {
		LOG_FATAL << "bind: " << strerror(errno);
	}
}

int Socket::accept(IpAddress& peer) {
	//todo:accept4
	socklen_t addrlen = static_cast<socklen_t>(sizeof (struct sockaddr));
	int connfd = ::accept(fd_, peer.getSockAddr(), &addrlen);
	setNonBlockAndCloseOnExec();

	if (connfd < 0) {
		//todo:
		LOG_FATAL << "accept:" << strerror(errno);
	}
	return connfd;
}

int Socket::fd() const {
	return fd_;
}

void Socket::setTcpNoDelay(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_FATAL << "setsockopt: " << strerror(errno);
	}
}

void Socket::setReuseAddr(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, IPPROTO_TCP, SO_REUSEADDR, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_FATAL << "setsockopt: " << strerror(errno);
	}
}

void Socket::setReusePort(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, IPPROTO_TCP, SO_REUSEPORT, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_FATAL << "setsockopt: " << strerror(errno);
	}
}

void Socket::setKeepAlive(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, IPPROTO_TCP, SO_KEEPALIVE, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_FATAL << "setsockopt: " << strerror(errno);
	}
}

void Socket::setNonBlockAndCloseOnExec() {
	int flags = ::fcntl(fd_, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(fd_, F_SETFL, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl" << strerror(errno);
	}

	flags = ::fcntl(fd_, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(fd_, F_SETFD, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl" << strerror(errno);
	}
}

}
