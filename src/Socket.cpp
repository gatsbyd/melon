#include "Address.h"
#include "Log.h"
#include "Hook.h"
#include "Socket.h"
#include "SchedulerThread.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>


namespace melon {
	
Socket::~Socket() {
	scheduler_->removeEvent(fd_);
	::close(fd_);
	LOG_DEBUG << "destroy socket:" << fd_;
}

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
	if (connfd < 0) {
		LOG_FATAL << "accept: " << strerror(errno);
	}
	SetNonBlockAndCloseOnExec(connfd);

	if (connfd < 0) {
		//todo: handle error
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

ssize_t Socket::read(void *buf, size_t count) {
	return ::read(fd_, buf, count);
}

ssize_t Socket::write(const void *buf, size_t count) {
	return ::write(fd_, buf, count);
}	

void Socket::shutdownWrite() {
	if (::shutdown(fd_, SHUT_WR) < 0) {
		LOG_ERROR << "socket:shutdownWrite:" << strerror(errno);
	}
}

void Socket::SetNonBlockAndCloseOnExec(int fd) {
	int flags = ::fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(fd, F_SETFL, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl: fd=" << fd << ", " << strerror(errno);
	}

	flags = ::fcntl(fd, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(fd, F_SETFD, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl: fd=" << fd << ","  << strerror(errno);
	}
}

int Socket::CreateSocket() {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		LOG_FATAL << "socket: " << strerror(errno);
	}

	SetNonBlockAndCloseOnExec(fd);
	return fd;
}

}
