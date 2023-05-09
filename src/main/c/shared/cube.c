#include <errno.h>
#include <signal.h>
#include <sys/mman.h>

#include "collection.h"
#include "cube.h"
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

__attribute__ ((constructor (102)))
static void __atfork(void);

static void __child(void);

__attribute__ ((constructor (102)))
static void __init(void);

static void __parent(void);

static void __prepare(void);

static int __access(const void *addr) {
	LOCK;

	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			SYSLOG(__lock(&cube->mutex));

			if (__from_ptr(cube->page.addr) <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + cube->page.size) {
				UNLOCK;

				if (cube->pid != __process()) {
					SYSLOG(__unlock(&cube->mutex));

					errno = EACCES;
					return -1;
				}

				if (cube->slices) {
					slice_t **__slices;
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(cube->page.addr) + slice->page.offset <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + slice->page.offset + slice->page.size) {
							if (slice->nlinks) {
								if (__from_ptr(cube->page.addr) + slice->offset <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + slice->offset + slice->size) {
									SYSLOG(__unlock(&cube->mutex));

									errno = EPERM;
									return -1;
								}
							}
						}
					}
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(cube->page.addr) + slice->page.offset <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + slice->page.offset + slice->page.size) {
							if (slice->nlinks) {
								if (__from_ptr(cube->page.addr) + slice->offset > __from_ptr(addr) || __from_ptr(addr) >= __from_ptr(cube->page.addr) + slice->offset + slice->size) {
									while (slice->nlinks) {
										if (__wait(&slice->cond, &cube->mutex)) {
											int errsv = errno;

											SYSLOG(__unlock(&cube->mutex));

											errno = errsv;
											return -1;
										}
									}

									SYSLOG(__unlock(&cube->mutex));

									return 0;
								}
							}
						}
					}
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(cube->page.addr) + slice->page.offset <= __from_ptr(addr) && __from_ptr(addr) < __from_ptr(cube->page.addr) + slice->page.offset + slice->page.size) {
							if (__unprotect(__to_ptr(__from_ptr(cube->page.addr) + slice->page.offset), slice->page.size)) {
								int errsv = errno;

								SYSLOG(__unlock(&cube->mutex));

								errno = errsv;
								return -1;
							} else {
								slice->version++;
							}
						}
					}
				}

				SYSLOG(__unlock(&cube->mutex));

				return 0;
			}

			SYSLOG(__unlock(&cube->mutex));
		}
	}

	UNLOCK;

	return SIGSEGV;
}

static void __action(int signal, siginfo_t *info, void *context) {
	if (SYSLOG_NEGATIVE(__access(info->si_addr))) {
		(*__default.sa_sigaction)(signal, info, context);
	}
}

static void __atfork(void) {
	SYSLOG(pthread_atfork(__prepare, __parent, __child));
}

int __attach(slice_t *slice) {
	cube_t *cube = slice->cube;

	if (cube->pid != __process()) {
		errno = EACCES;
		return -1;
	}

	SYSLOG(__lock(&cube->mutex));

	if (__protect(__to_ptr(__from_ptr(cube->page.addr) + slice->page.offset), slice->page.size)) {
		int errsv = errno;

		SYSLOG(__unlock(&cube->mutex));

		errno = errsv;
		return -1;
	}

	cube->nlinks++;

	slice->nlinks++;

	SYSLOG(__unlock(&cube->mutex));

	return 0;
}

static void __child(void) {
	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			SYSLOG(__protect(cube->page.addr, cube->page.size));

			SYSLOG(__unlock(&cube->mutex));
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

	SYSLOG(__lock(&cube->mutex));

	slice->nlinks--;

	SYSLOG(__broadcast(&slice->cond));

	if (!cube->nlinks--) {
		SYSLOG(__unlink(cube->id));

		if (cube->slices) {
			slice_t **__slices;
			for (__slices = cube->slices; *__slices != NULL; __slices++) {
				slice_t *slice = *__slices;

				SYSLOG(__destroy_cond(&slice->cond));

				SYSLOG(__free(slice, sizeof(slice_t)));
			}
		}

		__clear((void ***) &cube->slices);

		SYSLOG(__unlock(&cube->mutex));

		SYSLOG(__destroy_mutex(&cube->mutex));

		SYSLOG(__free(cube, sizeof(cube_t)));

		return 0;
	}

	SYSLOG(__unlock(&cube->mutex));

	return 0;
}

static void __init(void) {
	__inaccel.sa_flags = SA_SIGINFO;
	__inaccel.sa_sigaction = &__action;

	SYSLOG(sigaction(SIGSEGV, &__inaccel, &__default));
}

static void __parent(void) {
	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			SYSLOG(__unlock(&cube->mutex));
		}
	}

	UNLOCK;
}

static void __prepare(void) {
	LOCK;

	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			SYSLOG(__lock(&cube->mutex));
		}
	}
}

slice_t *__slice(const void *addr, size_t size) {
	LOCK;

	if (cubes) {
		cube_t **__cubes;
		for (__cubes = cubes; *__cubes != NULL; __cubes++) {
			cube_t *cube = *__cubes;

			SYSLOG(__lock(&cube->mutex));

			if (__from_ptr(cube->page.addr) <= __from_ptr(addr) && __from_ptr(addr) + size <= __from_ptr(cube->page.addr) + cube->page.size) {
				UNLOCK;

				if (cube->pid != __process()) {
					SYSLOG(__unlock(&cube->mutex));

					errno = EACCES;
					return NULL;
				}

				if (cube->slices) {
					slice_t **__slices;
					for (__slices = cube->slices; *__slices != NULL; __slices++) {
						slice_t *slice = *__slices;

						if (__from_ptr(addr) == __from_ptr(cube->page.addr) + slice->offset && size == slice->size) {
							SYSLOG(__unlock(&cube->mutex));

							return slice;
						}
					}
				}

				slice_t *slice = (slice_t *) __alloc(sizeof(slice_t), PRIVATE);
				if (!slice) {
					return NULL;
				}

				if (__init_cond(&slice->cond, PROCESS_PRIVATE)) {
					int errsv = errno;

					SYSLOG(__free(slice, sizeof(slice_t)));

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

					SYSLOG(__unlock(&cube->mutex));

					SYSLOG(__destroy_cond(&slice->cond));

					SYSLOG(__free(slice, sizeof(slice_t)));

					errno = errsv;
					return NULL;
				}

				SYSLOG(__unlock(&cube->mutex));

				return slice;
			}

			SYSLOG(__unlock(&cube->mutex));
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
		return NULL;
	}

	cube->pid = __process();

	if (__init_mutex(&cube->mutex, PROCESS_PRIVATE)) {
		int errsv = errno;

		SYSLOG(__free(cube, sizeof(cube_t)));

		errno = errsv;
		return NULL;
	}

	int fd = __link_open(cube->id);
	if (fd == -1) {
		int errsv = errno;

		SYSLOG(__destroy_mutex(&cube->mutex));

		SYSLOG(__free(cube, sizeof(cube_t)));

		errno = errsv;
		return NULL;
	}

	size = __ceil_pagesize(size);

	void *addr = __map(size, fd);
	if (addr == MAP_FAILED) {
		int errsv = errno;

		SYSLOG(__close(fd));

		SYSLOG(__destroy_mutex(&cube->mutex));

		SYSLOG(__free(cube, sizeof(cube_t)));

		errno = errsv;
		return NULL;
	}

	SYSLOG(__close(fd));

	cube->page.addr = addr;
	cube->page.size = size;

	LOCK;

	if (__set((void ***) &cubes, (void *) cube)) {
		int errsv = errno;

		UNLOCK;

		SYSLOG(__unmap(cube->page.addr, cube->page.size));

		SYSLOG(__unlink(cube->id));

		SYSLOG(__destroy_mutex(&cube->mutex));

		SYSLOG(__free(cube, sizeof(cube_t)));

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

				SYSLOG(__lock(&cube->mutex));

				if (__from_ptr(cube->page.addr) == __from_ptr(addr)) {
					__unset((void ***) &cubes, (void *) cube);

					UNLOCK;

					SYSLOG(__unmap(cube->page.addr, cube->page.size));

					if (cube->pid != __process() || !cube->nlinks--) {
						if (cube->pid == __process()) {
							SYSLOG(__unlink(cube->id));
						}

						if (cube->slices) {
							slice_t **__slices;
							for (__slices = cube->slices; *__slices != NULL; __slices++) {
								slice_t *slice = *__slices;

								SYSLOG(__destroy_cond(&slice->cond));

								SYSLOG(__free(slice, sizeof(slice_t)));
							}
						}

						__clear((void ***) &cube->slices);

						SYSLOG(__unlock(&cube->mutex));

						SYSLOG(__destroy_mutex(&cube->mutex));

						SYSLOG(__free(cube, sizeof(cube_t)));

						return;
					}

					SYSLOG(__unlock(&cube->mutex));

					return;
				}

				SYSLOG(__unlock(&cube->mutex));
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

				SYSLOG(__lock(&cube->mutex));

				if (__from_ptr(cube->page.addr) == __from_ptr(addr)) {
					UNLOCK;

					if (cube->pid != __process()) {
						SYSLOG(__unlock(&cube->mutex));

						errno = EACCES;
						return NULL;
					}

					if (new_size <= cube->page.size) {
						SYSLOG(__unlock(&cube->mutex));

						return cube->page.addr;
					}

					int fd = __reopen(cube->id);
					if (fd == -1) {
						int errsv = errno;

						SYSLOG(__unlock(&cube->mutex));

						errno = errsv;
						return NULL;
					}

					new_size = __ceil_pagesize(new_size);

					void *new_addr = __remap(cube->page.addr, cube->page.size, new_size, fd);
					if (new_addr == MAP_FAILED) {
						int errsv = errno;

						SYSLOG(__close(fd));

						SYSLOG(__unlock(&cube->mutex));

						errno = errsv;
						return NULL;
					}

					SYSLOG(__close(fd));

					cube->page.addr = new_addr;
					cube->page.size = new_size;

					SYSLOG(__unlock(&cube->mutex));

					return cube->page.addr;
				}

				SYSLOG(__unlock(&cube->mutex));
			}
		}

		UNLOCK;
	}

	errno = EADDRNOTAVAIL;
	return NULL;
}
