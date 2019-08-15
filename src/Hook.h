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

//read
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
extern read_func read_origin;

//write
typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
extern write_func write_origin;

}

#endif
