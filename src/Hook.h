#ifndef _MELON_HOOK_H_
#define _MELON_HOOK_H_

#include <sys/socket.h>

extern "C" {

typedef int (*accept_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern accept_func accept_origin;

typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
extern read_func read_origin;

typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
extern write_func write_origin;

}

#endif
