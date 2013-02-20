#ifndef __NET_IO_H__
#define __NET_IO_H__
#include <sys/types.h>
#define IO_BUF_SIZE 1024
typedef struct fd_rbuf
{
	int fd;
	char buf[IO_BUF_SIZE];
	char *out;
	ssize_t unread_num;
} fd_rbuf_t;

int fd_rbuf_init(fd_rbuf_t *rbufp, int fd);
ssize_t fd_rbuf_read(fd_rbuf_t *rbufp, void *buf, int n);
ssize_t fd_rbuf_readl(fd_rbuf_t *rbufp, void *buf, int n);
ssize_t writen(int fd, const void *buffer, size_t n);

int make_unblock(int fd);
#endif