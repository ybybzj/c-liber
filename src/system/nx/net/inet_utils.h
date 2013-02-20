#ifndef __INET_UTILS_H__
#define __INET_UTILS_H__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>



/**
 * inet_tcp_connect - create a tcp socket, and connects it to the address specified by host and serv_port
 * @host:			the string containing either a hostname or a numeric address, or loopback IP if is set to NULL.
 * @serv_port:		the string containing either a service name or a numeric port number
 * return:			sockt file descriptor on success, -1 on error
 * Note: typically used by tcp client.
 */
int inet_tcp_connect(const char *host, const char *serv_port);

/**
 * inet_tcp_listen - create a socket with given type, bound to the wildcard IP address on the port specified by serv_port and type
 * @host:			the string containing either a hostname or a numeric address, or wildcat IP if is set to NULL.
 * @serv_port:		the string containing either a service name or a numeric port number
 * @addrlen:		if not NULL, used to return the size of the socket address structure corresponding to the returned file descriptor 
 * return:			sockt file descriptor on success, -1 on error
 * Note: used by TCP server.
 */
#define LISTENQ 10
int inet_tcp_listen(const char *host, const char *serv_port, socklen_t *addrlen);

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
#define INET_ADDR_STR_LEN 4096
char *inet_atos(const struct sockaddr *addr, socklen_t addrlen, char *add_str, size_t addr_str_len);

char *peer_atos(int sockfd, char *addr_str, size_t addr_str_len);

char *local_atos(int sockfd, char *addr_str, size_t addr_str_len);

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
int fd_dump(int rfd, int wfd, void (*finish_cb)(int err, int rfd, int wfd, void *arg), void *arg);

/*--------------helpers for struct sockaddr---------------------------------------------------------*/
//Returns: 0 if saddr is assigned successfully, -1 is error occurs. note: cannot be used for unix domain socket
int make_sockaddr(int s_family, const char *s_presentation, uint16_t port, struct sockaddr *saddr, socklen_t slen);

struct addrinfo *info_from(const char *host, const char *serv, int family, int socktype, int flags);
#endif