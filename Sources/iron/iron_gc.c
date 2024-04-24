
#ifdef WITH_MINITS

#include "iron_gc.h"
#include <gc.h>

void *gc_alloc(size_t size) {
	return _gc_calloc(size, sizeof(uint8_t));
}

void *gc_global(void *ptr) {
	return _gc_make_static(ptr);
}

void *gc_realloc(void *ptr, size_t size) {
	return _gc_realloc(ptr, size);
}

void gc_free(void *ptr) {
	if (ptr != NULL) {
		_gc_free(ptr);
	}
}

void gc_pause() {
	_gc_pause();
}

void gc_resume() {
	_gc_resume();
}

void gc_run() {
	_gc_run();
}

void gc_start(void *bos) {
    _gc_start(bos);
}

void gc_stop() {
    _gc_stop();
}

#endif
