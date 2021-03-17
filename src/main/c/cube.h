#ifndef CUBE_H
#define CUBE_H

#include "self.h"

__BEGIN_DECLS

struct __cube;
struct __slice;

typedef struct __cube {
	pthread_mutex_t mutex;

	int nlinks;
	struct {
		void *addr;
		size_t size;
	} page;
	pid_t pid;

	struct __slice **slices;

	temp_t id;
} cube_t;

typedef struct __slice {
	pthread_mutex_t mutex;

	struct {
		off_t offset;
		size_t size;
	} page;

	struct __cube *cube;

	off_t offset;
	size_t size;
	unsigned int version;
} slice_t;

int __attach(slice_t *slice);

int __detach(slice_t *slice);

slice_t *__slice(const void *addr, size_t size);

__END_DECLS

#endif // CUBE_H
