#include "io.h"
#include "ring_buffer.h"
#include "../common.h"
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>


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


/**
 * fd_dump - stream data between two file descriptors, read data from first one and write data to the other
 *				until get EOF
 * @rfd:		the file descriptor from which data is read
 * @wfd:		the file descriptor to which data is write
 * @finish_cb:	function pointer to the callback function to invoke when all data is streamed or an error occured.
 * @arg:		argument pass to the callback function 
 * return:			0 on success, -1 on error
 * Note: 
 */
#define RBUF_LEN 4096
static inline int set_fd(int rfd, int wfd,fd_set *rset, fd_set *wset, ring_buffer_t *rbufp, int eof)
{
	int maxfd;
    FD_ZERO(rset);
    FD_ZERO(wset);

    if(eof == 0 && ring_buffer_free_space(rbufp))
        FD_SET(rfd, rset);


    if(ring_buffer_data_count(rbufp))
        FD_SET(wfd, wset);

    maxfd = rfd > wfd ? rfd : wfd;
    return maxfd + 1;
}
int fd_dump(int rfd, int wfd, void (*finish_cb)(int, int, int, void *), void *arg)
{
	
	ring_buffer_t *rbuf = NULL;
    fd_set rset, wset;
    check((rbuf = ring_buffer_create(RBUF_LEN)) != NULL, return -1);
    int eof = 0;
    int rfd_flags, wfd_flags;
    check((rfd_flags = make_unblock(rfd)) != -1, goto onerr);
    check((wfd_flags = make_unblock(wfd)) != -1, goto onerr);
    for(;;)
    {
    	errno = 0;
    	int maxfd = set_fd(rfd, wfd, &rset, &wset, rbuf,eof);
    	// check(select(maxfd, &rset, &wset, NULL, NULL) != -1, goto onerr);
    	if(select(maxfd, &rset, &wset, NULL, NULL) < 0)
    	{
    		if(errno == EINTR)
    		{
    			print_warn("signal caught!!!");
    			goto onerr;
    		}
    		else
    		{
    			print_err("select failed!");
    			goto onerr;
    		}
    	}

    	if(FD_ISSET(rfd, &rset))
    	{
    		int n = read(rfd, ring_buffer_write_pos(rbuf), ring_buffer_free_space(rbuf));
    		if(n > 0)
            {
                ring_buffer_commit_write(rbuf, n);
                FD_SET(wfd,&wset);// data available write to server
            }else{
                if(n == 0)
                {
                    eof = 1;	
                }
                if (n < 0 && (errno != EWOULDBLOCK || errno != EAGAIN) )
                {   
                    if(finish_cb != NULL)
                    	finish_cb(-1, rfd, wfd, arg);    
                    sentinel(goto onerr,"read from rfd %d failed!", rfd);
                }
            }
    	}
    	int data_count = 0;
    	if(FD_ISSET(wfd, &wset) && (data_count = ring_buffer_data_count(rbuf)))
    	{
    		int n = write(wfd, ring_buffer_read_pos(rbuf), data_count);
    		if(n >= 0)
    		{
    			ring_buffer_commit_read(rbuf, n);
    		}else
    		{
    			if(errno != EWOULDBLOCK || errno != EAGAIN)
    			{   
                    sentinel(goto onerr,"write to wfd %d failed!", wfd);
                }

    		}
    	}

    	if(eof && ring_buffer_data_count(rbuf) == 0)
        {
        	ring_buffer_free(rbuf);
        	if(finish_cb != NULL)
        		finish_cb(0, rfd, wfd, arg);    
        	break;
        }
    }

    check(fcntl(rfd, F_SETFL, rfd_flags) != -1, return -1);
    check(fcntl(wfd, F_SETFL, wfd_flags) != -1, return -1);
    
    return 0;
onerr:
	ring_buffer_free(rbuf);
	if(finish_cb != NULL)
        finish_cb(-1, rfd, wfd, arg);
    check(fcntl(rfd, F_SETFL, rfd_flags) != -1, return -1);
    check(fcntl(wfd, F_SETFL, wfd_flags) != -1, return -1);   
	return -1;
}