#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
typedef struct ring_buffer
{
	/*pointer to underlying buffer*/
	void *buf;  
	/*size of underlying buffer(in bytes)*/
	unsigned long size;

	/*both are byte offset*/
	unsigned long write_offset; 
	unsigned long read_offset;
}ring_buffer_t;

int ring_buffer_create(ring_buffer_t *rbuf, unsigned long size);

int ring_buffer_free(ring_buffer_t *rbuf);

void *ring_buffer_write_pos(ring_buffer_t *rbuf);

void ring_buffer_commit_write(ring_buffer_t *rbuf, unsigned long w_num);

void *ring_buffer_read_pos(ring_buffer_t *rbuf);

void ring_buffer_commit_read(ring_buffer_t *rbuf, unsigned long r_num);

unsigned long ring_buffer_data_count(ring_buffer_t *rbuf);

unsigned long ring_buffer_free_space(ring_buffer_t *rbuf);

void ring_buffer_reset(ring_buffer_t *rbuf);

void print_ring_buf(ring_buffer_t *rbuf);
#endif