#ifndef __NET_IO_H__
#define __NET_IO_H__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define DIR_MODE (FILE_MODE | S_IXUSR|S_IXGRP|S_IXOTH)

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

/*return constants that determine the file type: 
  S_IFREG  -> Regular file
  S_IFDIR  -> Directory
  S_IFCHR  -> Character device
  S_IFBLK  -> Block device
  S_IFIFO  -> FIFO or pipe
  S_IFSOCK -> Socket 
  S_IFLNK  -> Symbolic link
  -1 	   -> error happened
*/

enum fd_type
{
	FT_REG = (0x01<<0),
	FT_DIR = (0x01<<1),
	FT_CHR = (0x01<<2),
	FT_BLK = (0x01<<3),
	FT_FIFO = (0x01<<4),
	FT_SOCK = (0x01<<5),
	FT_LNK = (0x01<<6),
};

typedef uint16_t fd_type_t;
fd_type_t get_fd_type(int fd); 

#endif