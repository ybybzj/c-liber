#define _GNU_SOURCE
#include "ring_buffer.h"
#include "../common.h"
#include "../time.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#define RB_MAPFILE_NAME_PATTERN "/rbuf_map_file_%lld"
#define RB_MAPFILE_NAME_LEN 31

struct ring_buffer
{
	/*pointer to underlying buffer*/
	void *buf;  
	/*size of underlying buffer(in bytes)*/
	unsigned long size;

	/*both are byte offset*/
	unsigned long write_offset; 
	unsigned long read_offset;
	pthread_mutex_t mtx;
};


ring_buffer_t *ring_buffer_create(unsigned long size)
{
	ERR_CLEAN_INIT();
	int fd;
	ring_buffer_t *rbuf = NULL;

	check((rbuf = MALLOC(1,ring_buffer_t)) != NULL, goto onerr);
	ERR_CLEAN_PUSH(0,rbuf,rbuf);

	check_t(pthread_mutex_init(&rbuf->mtx,NULL),goto onerr);
	ERR_CLEAN_PUSH(1,&rbuf->mtx,rbuf_mtx);

	char map_file_name[RB_MAPFILE_NAME_LEN] = {'\0'};
	check(rbuf != NULL && size > 0, errno = EINVAL;goto onerr);
	size = (size + (1UL << 12) - 1) >> 12 << 12;
	rbuf->size = size;

	snprintf(map_file_name,RB_MAPFILE_NAME_LEN, RB_MAPFILE_NAME_PATTERN, (long long)rbuf);//every single rbuf need a different map file name
	// println("map_file_name: %s",map_file_name);
	check((fd = shm_open(map_file_name, O_RDWR|O_CREAT|O_EXCL, S_IRUSR | S_IWUSR)) != -1,goto onerr);
	check(shm_unlink(map_file_name) != -1, close(fd);goto onerr);
	
	check(ftruncate(fd, size) != -1, close(fd);goto onerr);
	rbuf->buf = mmap(NULL, rbuf->size<<1, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	
	if(mmap(rbuf->buf, rbuf->size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0) != rbuf->buf)
	{
		print_err("duplicating buffer first failed!");
		munmap (rbuf->buf, rbuf->size<<1);
		close(fd);
		goto onerr;
	}else if(mmap((char*)rbuf->buf + rbuf->size, rbuf->size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0) != ((char*)rbuf->buf + rbuf->size))
	{
		print_err("duplicating buffer second failed!");
		munmap (rbuf->buf, rbuf->size << 1);
		close(fd);
		goto onerr;
	}
	check(close(fd)!= -1, goto onerr);

	
	rbuf->write_offset = 0;
	rbuf->read_offset = 0;
	return rbuf;

onerr:
	ERR_CLEAN_BEGIN
		case 0:
			{
				free((ring_buffer_t*)cleanup_resource);
				break;
			}
		case 1:
			{
				pthread_mutex_destroy((pthread_mutex_t*)cleanup_resource);
				break;
			}
		ERR_CLEAN_END
		return NULL;
}

int ring_buffer_free(ring_buffer_t *rbuf)
{
	int ret = 0;
	if(rbuf == NULL) return ret;
	if(rbuf->buf != NULL)
		check(munmap (rbuf->buf, rbuf->size << 1) != -1, ret = -1);
	check_t(pthread_mutex_destroy(&rbuf->mtx), ret = -1);
	free(rbuf);
	return ret;
}


void *ring_buffer_write_pos(ring_buffer_t *rbuf)
{
	return (void*)((char*)rbuf->buf + rbuf->write_offset);
}

void ring_buffer_commit_write(ring_buffer_t *rbuf, unsigned long w_num)
{
	(void)pthread_mutex_lock(&rbuf->mtx);
	rbuf->write_offset += w_num;
	(void)pthread_mutex_unlock(&rbuf->mtx);
}

void *ring_buffer_read_pos(ring_buffer_t *rbuf)
{
	return (void*)((char*)rbuf->buf + rbuf->read_offset);
}

void ring_buffer_commit_read(ring_buffer_t *rbuf, unsigned long r_num)
{
	(void)pthread_mutex_lock(&rbuf->mtx);
	rbuf->read_offset += r_num;
	if(rbuf->read_offset >= rbuf->size)
	{
		rbuf->read_offset -= rbuf->size;
		rbuf->write_offset -= rbuf->size;
	}
	(void)pthread_mutex_unlock(&rbuf->mtx);
}

unsigned long ring_buffer_data_count(ring_buffer_t *rbuf)
{
	return rbuf->write_offset - rbuf->read_offset;
}

unsigned long ring_buffer_free_space(ring_buffer_t *rbuf)
{
	return rbuf->size - ring_buffer_data_count(rbuf);
}

void ring_buffer_reset(ring_buffer_t *rbuf)
{
	(void)pthread_mutex_lock(&rbuf->mtx);
	rbuf->write_offset = 0;
	rbuf->read_offset = 0;
	(void)pthread_mutex_unlock(&rbuf->mtx);
}

void print_ring_buf(ring_buffer_t *rbuf)
{
	if(rbuf != NULL)
	{
		println("ring_buffer: %p", rbuf->buf);
		println("       size: %lu", rbuf->size);
		println("   read_pos: %p", ring_buffer_read_pos(rbuf));	
		println("  write_pos: %p", ring_buffer_write_pos(rbuf));	
		println(" data_count: %lu", ring_buffer_data_count(rbuf));
		println(" free_space: %lu", ring_buffer_free_space(rbuf));	
	}
}