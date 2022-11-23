#include "shm.h"

__attribute__ ((visibility ("default")))
void *PyDataMemType_CallocFunc(size_t nelem, size_t elsize) {
	return inaccel_alloc(nelem * elsize);
}

__attribute__ ((visibility ("default")))
void PyDataMemType_FreeFunc(void *ptr, size_t size) {
	inaccel_free(ptr);
}

__attribute__ ((visibility ("default")))
void *PyDataMemType_MallocFunc(size_t size) {
	return inaccel_alloc(size);
}

__attribute__ ((visibility ("default")))
void *PyDataMemType_ReallocFunc(void *ptr, size_t new_size) {
	return inaccel_realloc(ptr, new_size);
}
