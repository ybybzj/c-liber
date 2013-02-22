#define _GNU_SOURCE
#include "ring_buffer.h"
#include "../common.h"
#include "../time.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>
#define RB_MAPFILE_NAME_PATTERN "/rbuf_map_file_%ld"
#define RB_MAPFILE_NAME_LEN 31




int ring_buffer_create(ring_buffer_t *rbuf, unsigned long size)
{
	int fd;
	long sufix_num;

	char map_file_name[RB_MAPFILE_NAME_LEN] = {'\0'};
	check(rbuf != NULL && size > 0, errno = EINVAL;return -1);
	size = (size + (1UL << 12) - 1) >> 12 << 12;
	rbuf->size = size;

	
	check((sufix_num = get_timestamp(TM_UNIT_MS)) != -1, return -1);
	snprintf(map_file_name,RB_MAPFILE_NAME_LEN, RB_MAPFILE_NAME_PATTERN, sufix_num);
	// println("map_file_name: %s",map_file_name);
	check((fd = shm_open(map_file_name, O_RDWR|O_CREAT|O_EXCL, S_IRUSR | S_IWUSR)) != -1,return -1);
	check(shm_unlink(map_file_name) != -1, close(fd);return -1);
	
	check(ftruncate(fd, size) != -1, close(fd);return -1);
	rbuf->buf = mmap(NULL, rbuf->size<<1, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	
	if(mmap(rbuf->buf, rbuf->size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0) != rbuf->buf)
	{
		print_err("duplicating buffer first failed!");
		munmap (rbuf->buf, rbuf->size<<1);
		close(fd);
		return -1;
	}else if(mmap((char*)rbuf->buf + rbuf->size, rbuf->size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0) != ((char*)rbuf->buf + rbuf->size))
	{
		print_err("duplicating buffer second failed!");
		munmap (rbuf->buf, rbuf->size << 1);
		close(fd);
		return -1;
	}
	check(close(fd)!= -1, return -1);
	rbuf->write_offset = 0;
	rbuf->read_offset = 0;
	return 0;
}

int ring_buffer_free(ring_buffer_t *rbuf)
{
	check(rbuf != NULL, errno = EINVAL;return -1);
	if(rbuf->buf == NULL) return 0;
	check(munmap (rbuf->buf, rbuf->size << 1) != -1, return -1);
	rbuf->buf = NULL;
	rbuf->size = 0;
	rbuf->write_offset = 0;
	rbuf->read_offset = 0;
	return 0;
}

void *ring_buffer_write_pos(ring_buffer_t *rbuf)
{
	return (void*)((char*)rbuf->buf + rbuf->write_offset);
}

void ring_buffer_commit_write(ring_buffer_t *rbuf, unsigned long w_num)
{
	rbuf->write_offset += w_num;
}

void *ring_buffer_read_pos(ring_buffer_t *rbuf)
{
	return (void*)((char*)rbuf->buf + rbuf->read_offset);
}

void ring_buffer_commit_read(ring_buffer_t *rbuf, unsigned long r_num)
{
	rbuf->read_offset += r_num;
	if(rbuf->read_offset >= rbuf->size)
	{
		rbuf->read_offset -= rbuf->size;
		rbuf->write_offset -= rbuf->size;
	}
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
	rbuf->write_offset = 0;
	rbuf->read_offset = 0;
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