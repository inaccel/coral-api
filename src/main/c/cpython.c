#include "shm.h"

__attribute__ ((visibility ("default")))
void *PyDataMem_AllocFunc(size_t size) {
	return inaccel_alloc(size);
}

__attribute__ ((visibility ("default")))
void PyDataMem_FreeFunc(void *ptr, size_t size) {
	inaccel_free(ptr);
}

__attribute__ ((visibility ("default")))
void *PyDataMem_ReallocFunc(void *ptr, size_t size) {
	return inaccel_realloc(ptr, size);
}

__attribute__ ((visibility ("default")))
void *PyDataMem_ZeroedAllocFunc(size_t nelems, size_t elsize) {
	return inaccel_alloc(nelems * elsize);
}
