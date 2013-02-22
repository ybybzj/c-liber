#ifndef __SIMPLE_STACK_H__
#define __SIMPLE_STACK_H__
#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct sstack_ent
{
	struct sstack_ent *top;
} sstack_ent_t;
typedef sstack_ent_t sstack_t;

#define INIT_SSTACK(name) {&(name)}
#define SSTACK(name) \
	sstack_t name = INIT_SSTACK(name)



#define sstack_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#endif

static inline int sstack_isempty(sstack_t *s)
{
	return s == s->top;
}

static inline void sstack_push(sstack_ent_t *sent, sstack_t *s)
{
	sent->top = s->top;
	s->top = sent;
}

static inline sstack_ent_t *sstack_pop(sstack_t *s)
{
	if(sstack_isempty(s))
		return NULL;
	sstack_ent_t *sent;
	sent = s->top;
	s->top = sent->top;
	return sent;
}