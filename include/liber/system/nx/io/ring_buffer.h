#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
typedef struct ring_buffer ring_buffer_t;

ring_buffer_t *ring_buffer_create(unsigned long size);

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