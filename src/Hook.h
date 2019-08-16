#ifndef _MELON_HOOK_H_
#define _MELON_HOOK_H_

#include <sys/socket.h>

extern "C" {

//sleep
typedef unsigned int (*sleep_func)(unsigned int seconds);
extern sleep_func sleep_origin;

//accept
typedef int (*accept_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern accept_func accept_origin;

//accept4
typedef int (*accept4_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);
extern accept4_func accept4_origin;

//read
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
extern read_func read_origin;

//readv
typedef ssize_t (*readv_func)(int fd, const struct iovec *iov, int iovcnt);
extern readv_func readv_origin;

//recv
typedef ssize_t (*recv_func)(int sockfd, void *buf, size_t len, int flags);
extern recv_func recv_origin;

//recvfrom
typedef ssize_t (*recvfrom_func)(int sockfd, void *buf, size_t len, int flags, 
				struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_func recvfrom_origin;

//recvmsg
typedef ssize_t (*recvmsg_func)(int sockfd, struct msghdr *msg, int flags);
extern recvmsg_func recvmsg_origin;


//write
typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
extern write_func write_origin;

}

#endif
