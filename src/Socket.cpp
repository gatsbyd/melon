#include "Address.h"
#include "Log.h"
#include "Socket.h"

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
	socklen_t addrlen = static_cast<socklen_t>(sizeof (struct sockaddr));
	int connfd = ::accept(fd_, peer.getSockAddr(), &addrlen);
	if (connfd < 0) {
		LOG_ERROR << "accept: " << strerror(errno);
	}
	return connfd;
}

int Socket::connect(IpAddress& server_addr) {
	socklen_t addrlen = static_cast<socklen_t>(sizeof (struct sockaddr));
	return ::connect(fd_, server_addr.getSockAddr(), addrlen);
}

int Socket::fd() const {
	return fd_;
}

void Socket::setTcpNoDelay(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_ERROR << "setsockopt: " << strerror(errno);
	}
}

void Socket::setReuseAddr(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_ERROR << "setsockopt: " << strerror(errno);
	}
}

void Socket::setReusePort(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_ERROR << "setsockopt: " << strerror(errno);
	}
}

void Socket::setKeepAlive(bool on) {
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, 
					&optval, static_cast<socklen_t>(sizeof optval));
	if (ret == -1) {
		LOG_ERROR << "setsockopt: " << strerror(errno);
	}
}

ssize_t Socket::read(void *buf, size_t count) {
	ssize_t n = ::read(fd_, buf, count);
	if (n < 0) {
		int err = GetSocketError(fd_);
		LOG_ERROR << "Socket::read() SO_ERROR = " << err << " " << strerror(err);
	}
	return n;
}

ssize_t Socket::readv(const struct iovec* iov, int iovcnt) {
	ssize_t n = ::readv(fd_, iov, iovcnt);
	if (n < 0) {
		int err = GetSocketError(fd_);
		LOG_ERROR << "Socket::readv() SO_ERROR = " << err << " " << strerror(err);
	}
	return n;
}

ssize_t Socket::write(const void *buf, size_t count) {
	ssize_t n = ::write(fd_, buf, count);
	if (n < 0) {
		int err = GetSocketError(fd_);
		LOG_ERROR << "Socket::write() SO_ERROR = " << err << " " << strerror(err);
	}
	return n;
}	

ssize_t Socket::writev(const struct iovec *iov, int iovcnt) {
	ssize_t n = ::write(fd_, iov, iovcnt);
	if (n < 0) {
		int err = GetSocketError(fd_);
		LOG_ERROR << "Socket::writev() SO_ERROR = " << err << " " << strerror(err);
	}
	return n;
}

void Socket::shutdownWrite() {
	if (::shutdown(fd_, SHUT_WR) < 0) {
		LOG_ERROR << "shutdownWrite:" << strerror(errno);
	}
}

void Socket::close() {
	::close(fd_);
}

void Socket::SetNonBlockAndCloseOnExec() {
	int flags = ::fcntl(fd_, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int ret = ::fcntl(fd_, F_SETFL, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl: fd=" << fd_ << ", " << strerror(errno);
	}

	flags = ::fcntl(fd_, F_GETFD, 0);
	flags |= FD_CLOEXEC;
	ret = ::fcntl(fd_, F_SETFD, flags);
	if (ret == -1) {
		LOG_FATAL << "fcntl: fd=" << fd_ << ","  << strerror(errno);
	}
}

Socket::Ptr Socket::CreateTcp() {
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		LOG_FATAL << "socket: " << strerror(errno);
	}

	return std::make_shared<Socket>(fd);
}

int Socket::GetSocketError(int sockfd)
{
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;	//Solaris
  }
  else
  {
	errno = optval;
    return optval;	//Berkeley
  }
}
}
