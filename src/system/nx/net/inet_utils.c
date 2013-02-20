#define _GNU_SOURCE
#include "inet_utils.h"
#include <errno.h>
#include "../common.h"
#include <sys/select.h>
#include "../io/io.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "../io/ring_buffer.h"



/**
 * inet_tcp_connect - create a tcp socket, and connects it to the address specified by host and serv_port
 * @host:			the string containing either a hostname or a numeric address, or loopback IP if is set to NULL.
 * @serv_port:		the string containing either a service name or a numeric port number
 * return:			sockt file descriptor on success, -1 on error
 * Note: typically used by tcp client.
 */
int inet_tcp_connect(const char *host, const char *serv_port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd;
//make hint
	memset(&hints, 0 ,sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	check(getaddrinfo(host, serv_port, &hints, &result) == 0, errno = ENOSYS; return -1);
	
	for( rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd < 0)
			continue;		//On error, try next address
		
		if(connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
		{
			break;
		} //connect sucess
		else{
			close(sfd);
		}

	}	

	freeaddrinfo(result);

	return (rp == NULL) ? -1 : sfd;
}


/**
 * inet_tcp_listen - create a socket with given type, bound to the wildcard IP address on the port specified by serv_port and type
 * @host:			the string containing either a hostname or a numeric address, or wildcat IP if is set to NULL.
 * @serv_port:		the string containing either a service name or a numeric port number
 * @addrlen:		if not NULL, used to return the size of the socket address structure corresponding to the returned file descriptor 
 * return:			sockt file descriptor on success, -1 on error
 * Note: used by TCP server.
 */
int inet_tcp_listen(const char *host, const char *serv_port, socklen_t *addrlen)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, optval;
//make hint
	memset(&hints, 0 ,sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    //Use wildcard IP address

	check(getaddrinfo(host, serv_port, &hints, &result) == 0, errno = ENOSYS; return -1);

	optval = 1;
	for(rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd == -1)
			continue;

		check(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != -1, 
			close(sfd); freeaddrinfo(result); return -1);
		

		if(bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;
		else
			close(sfd);
	}
	check(listen(sfd, LISTENQ) != -1, 
			close(sfd); freeaddrinfo(result); return -1);
		
	if(rp != NULL && addrlen != NULL)
	{
		*addrlen = rp->ai_addrlen;
	}
	freeaddrinfo(result);
	return (rp == NULL) ? -1 : sfd;
}

/**
 * inet_atos - build a null-terminated string containing the host and port number formed as "(hostname, port-number)" 
 *			   according to the given socket address structure
 * @addr:			the socket address structure
 * @addrlen:		the size of the socket address structure
 * @add_str:		points to the buffer used to store the returned string 
 * @addr_str_len:	the size of the buffer pointed by add_str
 * return:			add_str on success, NULL on error
 * Note: if returned string is exceed (addr_str_len-1), string will be truncated.
 */


char *inet_atos(const struct sockaddr *addr, socklen_t addrlen, char *addr_str, size_t addr_str_len)
{
	check(addr_str != NULL && addr_str_len-1 > 0, errno = EINVAL; return NULL);
	char host[NI_MAXHOST], serv_port[NI_MAXSERV];
	if(getnameinfo(addr, addrlen, host, NI_MAXHOST, serv_port, NI_MAXSERV, NI_NUMERICSERV|NI_NUMERICHOST) == 0) //get numeric serv_port
		snprintf(addr_str, addr_str_len, "(%.*s : %.*s)",NI_MAXHOST, host, NI_MAXSERV, serv_port);
	else
		snprintf(addr_str, addr_str_len, "(?UNKNOWN?)");
	addr_str[addr_str_len - 1] = '\0';
	return addr_str;
}

static char *_atos(int sockfd, char *addr_str, size_t addr_str_len, int (*fn)(int, struct sockaddr*, socklen_t*));

char *peer_atos(int sockfd, char *addr_str, size_t addr_str_len)
{
	return _atos(sockfd, addr_str, addr_str_len, getpeername);

}

char *local_atos(int sockfd, char *addr_str, size_t addr_str_len)
{
	return _atos(sockfd, addr_str, addr_str_len, getsockname);
}

static char *_atos(int sockfd, char *addr_str, size_t addr_str_len, int (*fn)(int, struct sockaddr*, socklen_t*))
{
	check(addr_str != NULL && addr_str_len-1 > 0, errno = EINVAL; return NULL);
	struct sockaddr_storage saddr;
	socklen_t slen;

	slen = sizeof(struct sockaddr_storage);
	if(fn == NULL || fn(sockfd, (struct sockaddr*)&saddr, &slen) != 0)
	{
		snprintf(addr_str, addr_str_len, "(?ERROR?)");
		return addr_str;
	}
	return inet_atos((struct sockaddr *)&saddr, slen, addr_str, addr_str_len);
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
	
	ring_buffer_t rbuf;
    fd_set rset, wset;
    ring_buffer_create(&rbuf,RBUF_LEN);
    int eof = 0;
    int rfd_flags, wfd_flags;
    check((rfd_flags = make_unblock(rfd)) != -1, goto onerr);
    check((wfd_flags = make_unblock(wfd)) != -1, goto onerr);
    for(;;)
    {
    	errno = 0;
    	int maxfd = set_fd(rfd, wfd, &rset, &wset, &rbuf,eof);
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
    		int n = read(rfd, ring_buffer_write_pos(&rbuf), ring_buffer_free_space(&rbuf));
    		if(n > 0)
            {
                ring_buffer_commit_write(&rbuf, n);
                FD_SET(wfd,&wset);// data available write to server
            }else{
                if(n == 0)
                {
                    eof = 1;	
                }
                if (n < 0 && errno != EWOULDBLOCK )
                {   
                    if(finish_cb != NULL)
                    	finish_cb(-1, rfd, wfd, arg);    
                    sentinel(goto onerr,"read from rfd %d failed!", rfd);
                }
            }
    	}
    	int data_count = 0;
    	if(FD_ISSET(wfd, &wset) && (data_count = ring_buffer_data_count(&rbuf)))
    	{
    		int n = write(wfd, ring_buffer_read_pos(&rbuf), data_count);
    		if(n >= 0)
    		{
    			ring_buffer_commit_read(&rbuf, n);
    		}else
    		{
    			if(errno != EWOULDBLOCK)
    			{   
                    sentinel(goto onerr,"write to wfd %d failed!", wfd);
                }

    		}
    	}

    	if(eof && ring_buffer_data_count(&rbuf) == 0)
        {
        	ring_buffer_free(&rbuf);
        	if(finish_cb != NULL)
        		finish_cb(0, rfd, wfd, arg);    
        	break;
        }
    }

    check(fcntl(rfd, F_SETFL, rfd_flags) != -1, return -1);
    check(fcntl(wfd, F_SETFL, wfd_flags) != -1, return -1);
    
    return 0;
onerr:
	ring_buffer_free(&rbuf);
	if(finish_cb != NULL)
        finish_cb(-1, rfd, wfd, arg);
    check(fcntl(rfd, F_SETFL, rfd_flags) != -1, return -1);
    check(fcntl(wfd, F_SETFL, wfd_flags) != -1, return -1);   
	return -1;
}



/*--------------------------------------------------------------------------*/
 #define MAKE_SADRR(saddrp,in_t,family,s_addr,s_port) do{\
	struct in_addr inaddr_any UNUSED = {(htonl(INADDR_ANY))};\
	struct in_addr inaddr_loopback UNUSED = {(htonl(INADDR_LOOPBACK))};\
	memset((saddrp),0,sizeof(struct sockaddr_##in_t));\
	(saddrp)->s##in_t##_family = family;\
	if((s_addr) == NULL){\
		(saddrp)->s##in_t##_addr = in_t##addr_loopback;\
	}else if(strlen((s_addr)) == 0)\
	{\
		(saddrp)->s##in_t##_addr = in_t##addr_any;\
	}else\
		check(inet_pton(family,(s_addr),&(saddrp)->s##in_t##_addr) > 0, return -1);\
	(saddrp)->s##in_t##_port = htons((uint16_t)(s_port));\
}while(0)

int make_sockaddr(int s_family, const char *s_str, uint16_t port, struct sockaddr *saddr, socklen_t slen)
{
	switch(s_family)
	{
		case AF_INET:
		{
			struct sockaddr_in *saddr_in = (struct sockaddr_in *)saddr;
			check(slen >= sizeof(struct sockaddr_in) && saddr != NULL, errno = EINVAL; return -1);
			MAKE_SADRR(saddr_in,in,AF_INET,s_str,port);
			return 0;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 *saddr_in6 = (struct sockaddr_in6 *)saddr;
			check(slen >= sizeof(struct sockaddr_in6) && saddr != NULL, errno = EINVAL; return -1);
			MAKE_SADRR(saddr_in6,in6,AF_INET6,s_str,port);
			return 0;
		}
		default:
		{
			errno = EINVAL;
			return -1;
		}
	}
}


struct addrinfo *info_from(const char *host, const char *serv, int family, int socktype, int flags)
{
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME|flags; //always return canonical name
	hints.ai_family = family;
	hints.ai_socktype = socktype;

	check(getaddrinfo(host, serv, &hints, &res) == 0, return NULL);
	return res;
}