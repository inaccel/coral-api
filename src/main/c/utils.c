#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <ftw.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "utils.h"

void *__alloc(size_t size, int flag) {
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | flag, -1, 0);
	if (addr == MAP_FAILED) {
		return NULL;
	} else {
		return addr;
	}
}

int __broadcast(pthread_cond_t *cond) {
	return pthread_cond_broadcast(cond);
}

uintptr_t __ceil_pagesize(uintptr_t ptr) {
	return (ptr + sysconf(_SC_PAGESIZE) - 1) & ~(sysconf(_SC_PAGESIZE) - 1);
}

int __daemon(void *(*start_routine) (void *), void *arg) {
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int error = pthread_create(&thread, &attr, start_routine, arg);
	pthread_attr_destroy(&attr);
	return error;
}

int __destroy_cond(pthread_cond_t *cond) {
	return pthread_cond_destroy(cond);
}

int __destroy_mutex(pthread_mutex_t *mutex) {
	return pthread_mutex_destroy(mutex);
}

void __exit(int status) {
	_exit(status);
}

uintptr_t __floor_pagesize(uintptr_t ptr) {
	return ptr & ~(sysconf(_SC_PAGESIZE) - 1);
}

int __fork(void) {
	return syscall(SYS_fork);
}

int __free(void *addr, size_t size) {
	return munmap(addr, size);
}

uintptr_t __from_ptr(const void *addr) {
	return (uintptr_t) addr;
}

int __init_cond(pthread_cond_t *cond, int p) {
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setpshared(&attr, p);
	int error = pthread_cond_init(cond, &attr);
	pthread_condattr_destroy(&attr);
	return error;
}

int __init_mutex(pthread_mutex_t *mutex, int p) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, p);
	int error = pthread_mutex_init(mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	return error;
}

int __lock(pthread_mutex_t *mutex) {
	return pthread_mutex_lock(mutex);
}

void *__map(size_t size, int fd) {
	if (!ftruncate(fd, size)) {
		return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	} else {
		return MAP_FAILED;
	}
}

int __perm(pid_t pid) {
	return (kill(pid, 0) && errno == EPERM);
}

pid_t __process(void) {
	return getpid();
}

int __protect(void *addr, size_t size) {
	return mprotect(addr, size, PROT_READ);
}

void *__realloc(void *old_addr, size_t new_size) {
	void *new_addr = mremap(old_addr, 0, new_size, MREMAP_MAYMOVE);
	if (new_addr == MAP_FAILED) {
		return NULL;
	} else {
		return new_addr;
	}
}

void *__remap(void *old_addr, size_t new_size, int fd) {
	if (!ftruncate(fd, new_size)) {
		return mremap(old_addr, 0, new_size, MREMAP_MAYMOVE);
	} else {
		return MAP_FAILED;
	}
}

int __remove(const char *pathname) {
	return nftw(pathname, (__nftw_func_t) remove, 1, FTW_DEPTH);
}

int __session(pid_t pid) {
	if (setsid() == -1) {
		return -1;
	}
	int fd;
	for (fd = 0; fd < sysconf(_SC_OPEN_MAX) - 1; fd++) {
		close(fd);
	}
	char comm[16];
	if (sprintf(comm, "inaccel/%i", pid) < 0) {
		return -1;
	}
	return prctl(PR_SET_NAME, comm);
}

int __signal(pthread_cond_t *cond) {
	return pthread_cond_signal(cond);
}

void __sleep(unsigned int seconds) {
	sleep(seconds);
}

int __srch(pid_t pid) {
	return (kill(pid, 0) && errno == ESRCH);
}

void __syslog(const char *file, int line) {
	int errsv = errno;
	openlog("coral-api", LOG_CONS | LOG_NDELAY | LOG_PID, LOG_USER);
	syslog(LOG_ERR, "%s:%d | %s", file, line, strerror(errsv));
	closelog();
	errno = errsv;
}

int __timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
	return pthread_cond_timedwait(cond, mutex, abstime);
}

void *__to_ptr(uintptr_t ptr) {
	return (void *) ptr;
}

int __unlock(pthread_mutex_t *mutex) {
	return pthread_mutex_unlock(mutex);
}

int __unmap(void *addr, size_t size) {
	return munmap(addr, size);
}

int __unprotect(void *addr, size_t size) {
	return mprotect(addr, size, PROT_READ | PROT_WRITE);
}

int __wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	return pthread_cond_wait(cond, mutex);
}
