#include "ev_emitter.h"
#include "dbg.h"

typedef struct 
{
	int once;
	emitter_callback cb;
}ev_cb_s;


static void free_cb(ev_cb_s *cbsp)
{
	if(cbsp != NULL)
		free(cbsp);
}

int ev_emitter_init(ev_emitter_t *emitter)
{
	check(emitter != NULL, errno = EINVAL;return -1);
	check(tst_init(&emitter->ev_cb_map) != -1,return -1);
	return 0;
}
int ev_emitter_on(ev_emitter_t *emitter, const char *ev, emitter_callback cb, emitter_callback *old_cb)
{
	check(emitter != NULL && ev != NULL, errno = EINVAL;return -1);
	ev_cb_s *cbsp;
	if(tst_search(emitter->ev_cb_map, ev, (void *)&cbsp) > 0)
	{
		*old_cb = cbsp->cb;
		cbsp->cb = cb;
		cbsp->once = 0;
	}else
	{
		check((cbsp = (ev_cb_s*)malloc(sizeof(ev_cb_s))) != NULL, return -1);
		cbsp->cb = cb;
		cbsp->once = 0;
		check(tst_insert(&emitter->ev_cb_map, ev, (void *)cbsp, NULL) != -1, return -1);
	}
	
	return 0;
}

int ev_emitter_once(ev_emitter_t *emitter, const char *ev, emitter_callback cb, emitter_callback *old_cb)
{
	check(emitter != NULL && ev != NULL, errno = EINVAL;return -1);
	ev_cb_s *cbsp;
	if(tst_search(emitter->ev_cb_map, ev, (void *)&cbsp) > 0)
	{
		*old_cb = cbsp->cb;
		cbsp->cb = cb;
		cbsp->once = 1;
	}else
	{
		check((cbsp = (ev_cb_s*)malloc(sizeof(ev_cb_s))) != NULL, return -1);
		cbsp->cb = cb;
		cbsp->once = 1;
		check(tst_insert(&emitter->ev_cb_map, ev, (void *)cbsp, NULL) != -1, return -1);
	}
	
	return 0;
}
int ev_emitter_off(ev_emitter_t *emitter, const char *ev, emitter_callback *cb)
{
	check(emitter != NULL && ev != NULL, errno = EINVAL;return -1);
	ev_cb_s *cbsp;
	if(tst_search(emitter->ev_cb_map, ev, (void *)&cbsp) > 0)
	{
		*cb = cbsp->cb;
		cbsp->cb = NULL;
		
	}
	return 0;
}

int ev_emitter_emit_f(ev_emitter_t *emitter, const char *ev, ...)
{
	check(emitter != NULL && ev != NULL, errno = EINVAL;return -1);
	ev_cb_s *cbsp;
	if(tst_search(emitter->ev_cb_map, ev, (void *)&cbsp) > 0 && cbsp != NULL && cbsp->cb != NULL)
	{
		arg_data_t arguments[ARGUMENT_MAX];
		int arg_len;
		va_list argList;
		va_start(argList,ev);
		arg_len = make_arguments(argList, arguments, ARGUMENT_MAX);
		va_end(argList);
		cbsp->cb(arg_len, arguments);
		if(cbsp->once)
			cbsp->cb = NULL;
	}
	return 0;
}

void ev_emitter_destroy(ev_emitter_t *emitter)
{
	if(emitter != NULL)
	{
		tst_destroy(emitter->ev_cb_map,(void (*)(void *))free_cb);
	}
}