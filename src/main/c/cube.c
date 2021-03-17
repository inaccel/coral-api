#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/user.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>

#include "collection.h"
#include "cube.h"
#include "debug.h"
#include "self.h"
#include "shm.h"
#include "utils.h"

static pthread_mutex_t it = PTHREAD_MUTEX_INITIALIZER;

#define LOCK pthread_mutex_lock(&it)
#define UNLOCK pthread_mutex_unlock(&it)

static cube_t **cubes;

static struct sigaction __default, __inaccel;

static int __access(const void *addr);

static void __action(int signal, siginfo_t *info, void *context);

static void __atfork();

static void __child();

static void __init();

static void __parent();

static void __prepare();

static int __access(const void *addr) {
	LOCK;

	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			__lock(&cube->mutex);

			if (__from_ptr(cube->page.addr) <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + cube->page.size) {
				UNLOCK;

				if (cube->pid != __process()) {
					__unlock(&cube->mutex);

					errno = EACCES;
					return -1;
				}

				if (cube->slices) {
					slice_t **__slices;
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(cube->page.addr) + slice->page.offset <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + slice->page.offset + slice->page.size) {
							if (__unprotect(__to_ptr(__from_ptr(cube->page.addr) + slice->page.offset), slice->page.size)) {
								int errsv = errno;

								__unlock(&cube->mutex);

								errno = errsv;
								return -1;
							} else {
								slice->version++;
							}
						}
					}
				}

				__unlock(&cube->mutex);

				return 0;
			}

			__unlock(&cube->mutex);
		}
	}

	UNLOCK;

	errno = EADDRNOTAVAIL;
	return -1;
}

static void __action(int signal, siginfo_t *info, void *context) {
	if (__access(info->si_addr)) {
		(*__default.sa_sigaction)(signal, info, context);
	}
}

__attribute__ ((constructor))
static void __atfork() {
	if (pthread_atfork(__prepare, __parent, __child)) {
		PERROR("__atfork: pthread_atfork");
	}
}

int __attach(slice_t *slice) {
	cube_t *cube = slice->cube;

	if (cube->pid != __process()) {
		errno = EACCES;
		return -1;
	}

	__lock(&cube->mutex);

	cube->nlinks++;

	if (__protect(__to_ptr(__from_ptr(cube->page.addr) + slice->page.offset), slice->page.size)) {
		int errsv = errno;

		PERROR("__attach: __protect");

		errno = errsv;
		return -1;
	}

	__unlock(&cube->mutex);

	return 0;
}

static void __child() {
	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			if (__protect(cube->page.addr, cube->page.size)) {
				PERROR("__child: __protect");
			}

			__unlock(&cube->mutex);
		}
	}
	UNLOCK;
}

int __detach(slice_t *slice) {
	cube_t *cube = slice->cube;

	if (cube->pid != __process()) {
		errno = EACCES;
		return -1;
	}

	__lock(&cube->mutex);

	if (!cube->nlinks--) {
		if (__unlink(cube->id)) {
			PERROR("__detach: __unlink");
		}

		if (cube->slices) {
			slice_t **__slices;
			for (__slices = cube->slices; *__slices != NULL; __slices++) {
				slice_t *slice = *__slices;

				if (__destroy_mutex(&slice->mutex)) {
					PERROR("inaccel_free: __destroy_mutex");
				}

				if (__free(slice, sizeof(slice_t))) {
					PERROR("inaccel_free: __free");
				}
			}
		}

		__clear((void ***) &cube->slices);

		__unlock(&cube->mutex);

		if (__destroy_mutex(&cube->mutex)) {
			PERROR("__detach: __destroy_mutex");
		}

		if (__free(cube, sizeof(cube_t))) {
			PERROR("__detach: __free");
		}

		return 0;
	}

	__unlock(&cube->mutex);

	return 0;
}

__attribute__ ((constructor))
static void __init() {
	__inaccel.sa_flags = SA_SIGINFO;
	__inaccel.sa_sigaction = &__action;

	if (sigaction(SIGSEGV, &__inaccel, &__default)) {
		PERROR("__init: sigaction");
	}
}

static void __parent() {
	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			__unlock(&cube->mutex);
		}
	}
	UNLOCK;
}

static void __prepare() {
	LOCK;
	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			__lock(&cube->mutex);
		}
	}
}

slice_t *__slice(const void *addr, size_t size) {
	LOCK;

	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			__lock(&cube->mutex);

			if (__from_ptr(cube->page.addr) <= __from_ptr(addr) && __from_ptr(addr) + size <= __from_ptr(cube->page.addr) + cube->page.size) {
				UNLOCK;

				if (cube->pid != __process()) {
					__unlock(&cube->mutex);

					errno = EACCES;
					return NULL;
				}

				if (cube->slices) {
					slice_t **__slices;
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(addr) == __from_ptr(cube->page.addr) + slice->offset && size == slice->size) {
							__unlock(&cube->mutex);

							return slice;
						}
					}
				}

				slice_t *slice = (slice_t *) __alloc(sizeof(slice_t), PRIVATE);
				if (!slice) {
					int errsv = errno;

					PERROR("__slice: __alloc");

					errno = errsv;
					return NULL;
				}

				if (__init_mutex(&slice->mutex, PROCESS_PRIVATE)) {
					int errsv = errno;

					if (__free(slice, sizeof(slice_t))) {
						PERROR("__slice: __free");
					}

					errno = errsv;
					return NULL;
				}

				slice->cube = cube;

				slice->offset = __from_ptr(addr) - __from_ptr(cube->page.addr);
				slice->size = size;

				slice->page.offset = __floor_pagesize(__from_ptr(addr)) - __from_ptr(cube->page.addr);
				slice->page.size = __ceil_pagesize(slice->offset - slice->page.offset + slice->size);

				if (__set((void ***) &cube->slices, (void *) slice)) {
					int errsv = errno;

					__unlock(&cube->mutex);

					PERROR("__slice: __set");

					if (__destroy_mutex(&slice->mutex)) {
						PERROR("__slice: __destroy_mutex");
					}

					if (__free(slice, sizeof(slice_t))) {
						PERROR("__slice: __free");
					}

					errno = errsv;
					return NULL;
				}

				__unlock(&cube->mutex);

				return slice;
			}

			__unlock(&cube->mutex);
		}
	}

	UNLOCK;

	errno = EADDRNOTAVAIL;
	return NULL;
}

__attribute__ ((visibility ("default")))
void *inaccel_alloc(size_t size) {
	if (!size) {
		return NULL;
	}

	cube_t *cube = (cube_t *) __alloc(sizeof(cube_t), PRIVATE);
	if (!cube) {
		int errsv = errno;

		PERROR("inaccel_alloc: __alloc");

		errno = errsv;
		return NULL;
	}

	cube->pid = __process();

	if (__init_mutex(&cube->mutex, PROCESS_PRIVATE)) {
		int errsv = errno;

		PERROR("inaccel_alloc: __init_mutex");

		if (__free(cube, sizeof(cube_t))) {
			PERROR("inaccel_alloc: __free");
		}

		errno = errsv;
		return NULL;
	}

	int fd = __link_open(cube->id);
	if (fd == -1) {
		int errsv = errno;

		PERROR("inaccel_alloc: __link");

		if (__destroy_mutex(&cube->mutex)) {
			PERROR("inaccel_alloc: __destroy_mutex");
		}

		if (__free(cube, sizeof(cube_t))) {
			PERROR("inaccel_alloc: __free");
		}

		errno = errsv;
		return NULL;
	}

	size = __ceil_pagesize(size);

	void *addr = __map(size, fd);
	if (addr == MAP_FAILED) {
		int errsv = errno;

		PERROR("inaccel_alloc: __map");

		if (__close(fd)) {
			PERROR("inaccel_alloc: __close");
		}

		if (__destroy_mutex(&cube->mutex)) {
			PERROR("inaccel_alloc: __destroy_mutex");
		}

		if (__free(cube, sizeof(cube_t))) {
			PERROR("inaccel_alloc: __free");
		}

		errno = errsv;
		return NULL;
	}

	if (__close(fd)) {
		PERROR("inaccel_alloc: close");
	}

	cube->page.addr = addr;
	cube->page.size = size;

	LOCK;

	if (__set((void ***) &cubes, (void *) cube)) {
		int errsv = errno;

		UNLOCK;

		PERROR("inaccel_alloc: __set");

		if (__unmap(cube->page.addr, cube->page.size)) {
			PERROR("inaccel_alloc: __unmap");
		}

		if (__unlink(cube->id)) {
			PERROR("inaccel_alloc: __unlink");
		}

		if (__destroy_mutex(&cube->mutex)) {
			PERROR("inaccel_alloc: __destroy_mutex");
		}

		if (__free(cube, sizeof(cube_t))) {
			PERROR("inaccel_alloc: __free");
		}

		errno = errsv;
		return NULL;
	}

	UNLOCK;

	return cube->page.addr;
}

__attribute__ ((visibility ("default")))
void inaccel_free(void *addr) {
	if (addr) {
		LOCK;

		if (cubes) {
			cube_t **__cubes;
			for (__cubes = cubes; *__cubes != NULL; __cubes++) {
				cube_t *cube = *__cubes;

				__lock(&cube->mutex);

				if (__from_ptr(cube->page.addr) == __from_ptr(addr)) {
					__unset((void ***) &cubes, (void *) cube);

					UNLOCK;

					if (__unmap(cube->page.addr, cube->page.size)) {
						PERROR("inaccel_free: __unmap");
					}

					if (cube->pid != __process() || !cube->nlinks--) {
						if (cube->pid == __process()) {
							if (__unlink(cube->id)) {
								PERROR("inaccel_free: __unlink");
							}
						}

						if (cube->slices) {
							slice_t **__slices;
							for (__slices = cube->slices; *__slices != NULL; __slices++) {
								slice_t *slice = *__slices;

								if (__destroy_mutex(&slice->mutex)) {
									PERROR("inaccel_free: __destroy_mutex");
								}

								if (__free(slice, sizeof(slice_t))) {
									PERROR("inaccel_free: __free");
								}
							}
						}

						__clear((void ***) &cube->slices);

						__unlock(&cube->mutex);

						if (__destroy_mutex(&cube->mutex)) {
							PERROR("inaccel_free: __destroy_mutex");
						}

						if (__free(cube, sizeof(cube_t))) {
							PERROR("inaccel_free: __free");
						}

						return;
					}

					__unlock(&cube->mutex);

					return;
				}

				__unlock(&cube->mutex);
			}
		}

		UNLOCK;
	}
}

__attribute__ ((visibility ("default")))
void *inaccel_realloc(void *addr, size_t new_size) {
	if (!addr) {
		return inaccel_alloc(new_size);
	} else if (!new_size){
		inaccel_free(addr);
	} else {
		LOCK;

		if (cubes) {
			cube_t **__cubes;
			for (__cubes = cubes; *__cubes != NULL; __cubes++) {
				cube_t *cube = *__cubes;

				__lock(&cube->mutex);

				if (__from_ptr(cube->page.addr) == __from_ptr(addr)) {
					UNLOCK;

					if (cube->pid != __process()) {
						__unlock(&cube->mutex);

						errno = EACCES;
						return NULL;
					}

					if (new_size <= cube->page.size) {
						__unlock(&cube->mutex);

						return cube->page.addr;
					}

					int fd = __reopen(cube->id);
					if (fd == -1) {
						int errsv = errno;

						PERROR("inaccel_realloc: __reopen");

						__unlock(&cube->mutex);

						errno = errsv;
						return NULL;
					}

					new_size = __ceil_pagesize(new_size);

					void *new_addr = __remap(cube->page.addr, new_size, fd);
					if (new_addr == MAP_FAILED) {
						int errsv = errno;

						PERROR("inaccel_realloc: __remap");

						if (__close(fd)) {
							PERROR("inaccel_realloc: __close");
						}

						__unlock(&cube->mutex);

						errno = errsv;
						return NULL;
					}

					if (__close(fd)) {
						PERROR("inaccel_realloc: __close");
					}

					cube->page.addr = new_addr;
					cube->page.size = new_size;

					__unlock(&cube->mutex);

					return cube->page.addr;
				}

				__unlock(&cube->mutex);
			}
		}

		UNLOCK;
	}

	errno = EADDRNOTAVAIL;
	return NULL;
}
