#include "io.h"
#include "../common.h"
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


int fd_rbuf_init(fd_rbuf_t *rbufp, int fd)
{
	check(rbufp != NULL, errno = EINVAL;return -1);
	memset(rbufp, '\0', sizeof(struct fd_rbuf));
	rbufp->fd = fd;
	rbufp->out = rbufp->buf;
	rbufp->unread_num = 0;
	return 0;
}


ssize_t fd_rbuf_read(fd_rbuf_t *fd_rbuf, void *buf, int n)
{
	check(buf != NULL && n > 0,errno = EINVAL;return -1);

	char *bufp = buf;
	ssize_t totRead = 0;
	for(;;)
	{	

		if(fd_rbuf->unread_num <= 0)
		{
			fd_rbuf->out = fd_rbuf->buf;
			fd_rbuf->unread_num = read(fd_rbuf->fd,fd_rbuf->buf,IO_BUF_SIZE);

			if(fd_rbuf->unread_num == -1 )
			{
				if(errno == EINTR)
					continue;
				else 
				{
					print_err("[fd_rbuf_read]: read from fd(%d)",fd_rbuf->fd);
					return -1;
				}
			}else if(fd_rbuf->unread_num == 0)
			{
				break;
			}
		}
			
		while( n > 0 && fd_rbuf->unread_num > 0)
		{
			*bufp++ = *(fd_rbuf->out++);
			totRead++;
			n--;
			fd_rbuf->unread_num--;
		}		
		if( n - 1 > 0 && fd_rbuf->unread_num <= 0)
			continue;
		else
			break;
	}
	return totRead;
}

ssize_t fd_rbuf_readl(fd_rbuf_t *fd_rbuf, void *buf, int n)
{

	

	check(buf != NULL && n > 0,errno = EINVAL;return -1);

	memset(buf,'\0',n);

	char *bufp = buf;
	for(;;)
	{	

		if(fd_rbuf->unread_num <= 0)
		{
			fd_rbuf->out = fd_rbuf->buf;
			fd_rbuf->unread_num = read(fd_rbuf->fd,fd_rbuf->buf,IO_BUF_SIZE);

			if(fd_rbuf->unread_num == -1 )
			{
				if(errno == EINTR)
					continue;
				else 
				{
					print_err("[fd_rbuf_readl]: read from fd(%d)",fd_rbuf->fd);
					return -1;
				}
			}else if(fd_rbuf->unread_num == 0)
			{
				break;
			}
		}
			
		while( n - 1 > 0 && *(fd_rbuf->out) != '\n' && fd_rbuf->unread_num > 0)
		{
			*bufp++ = *(fd_rbuf->out++);
			n--;
			fd_rbuf->unread_num--;
		}		
		if( n - 1 > 0 && *(fd_rbuf->out) != '\n' && fd_rbuf->unread_num <= 0)
			continue;
		else if(*(fd_rbuf->out) == '\n')
		{
			fd_rbuf->out++;
			fd_rbuf->unread_num--;
			break;
		}else
			break;
	}
	return strlen((char*)buf);
}


ssize_t writen(int fd, const void *buffer, size_t n)
{
	ssize_t numWritten;
	size_t totWritten;
	const char *buf = buffer;

	for(totWritten = 0; totWritten < n;)
	{
		numWritten = write(fd, buf, n - totWritten);

		if(numWritten <= 0)
		{
			if(numWritten == -1 && errno == EINTR)
				continue;
			else
				return -1;
		}
		totWritten += numWritten;
		buf += numWritten;
	}
	return totWritten;

}

int make_unblock(int fd)
{
	int flags;
	check((flags = fcntl(fd, F_GETFL, 0)) != -1, return -1);
	check(fcntl(fd, F_SETFL, flags|O_NONBLOCK) != -1, return -1);
	return flags;
}