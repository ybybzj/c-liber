#ifndef __EVENT_EMITTER_H__
#define __EVENT_EMITTER_H__
#include "arguments.h"
#include "ds/tsearch_tree.h"
typedef void (*ev_callback)(int arg_len, arg_data_t arguments[static arg_len]);
typedef struct {
	tstree_t ev_cb_map;
}ev_emitter_t;

int ev_emitter_init(ev_emitter_t *emitter);

int ev_emitter_on(ev_emitter_t *emitter, const char *ev, ev_callback cb, ev_callback *old_cb);
int ev_emitter_once(ev_emitter_t *emitter, const char *ev, ev_callback cb, ev_callback *old_cb);
int ev_emitter_off(ev_emitter_t *emitter, const char *ev, ev_callback *cb);

int ev_emitter_emit_f(ev_emitter_t *emitter, const char *ev, ...);
#define ev_emitter_emit(emitter,ev,...) (ev_emitter_emit_f(emitter,ev,##__VA_ARGS__,finarg()))

void ev_emitter_destroy(ev_emitter_t *emitter);
#endif