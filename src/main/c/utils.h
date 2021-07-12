#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdint.h>

#define PRIVATE MAP_PRIVATE

#define PROCESS_PRIVATE PTHREAD_PROCESS_PRIVATE

#define PROCESS_SHARED PTHREAD_PROCESS_SHARED

#define SHARED MAP_SHARED

#define SYSLOG(...) ({ \
	int error = __VA_ARGS__; \
	if (error) __syslog(__FILE__, __LINE__); \
	error; \
})

#define SYSLOG_NEGATIVE(...) ({ \
	int error = __VA_ARGS__; \
	if (error < 0) __syslog(__FILE__, __LINE__); \
	error; \
})

__BEGIN_DECLS

void *__alloc(size_t size, int flag);

int __broadcast(pthread_cond_t *cond);

uintptr_t __ceil_pagesize(uintptr_t ptr);

int __daemon(void *(*start_routine) (void *), void *arg);

int __destroy_cond(pthread_cond_t *cond);

int __destroy_mutex(pthread_mutex_t *mutex);

void __exit(int status);

uintptr_t __floor_pagesize(uintptr_t ptr);

int __fork(void);

int __free(void *addr, size_t size);

uintptr_t __from_ptr(const void *addr);

int __init_cond(pthread_cond_t *cond, int p);

int __init_mutex(pthread_mutex_t *mutex, int p);

int __lock(pthread_mutex_t *mutex);

void *__map(size_t size, int fd);

int __perm(pid_t pid);

pid_t __process(void);

int __protect(void *addr, size_t size);

void *__realloc(void *old_addr, size_t new_size);

void *__remap(void *old_addr, size_t new_size, int fd);

int __remove(const char *pathname);

int __session(pid_t pid);

int __signal(pthread_cond_t *cond);

void __sleep(unsigned int seconds);

int __srch(pid_t pid);

void __syslog(const char *file, int line);

int __timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

void *__to_ptr(uintptr_t ptr);

int __unlock(pthread_mutex_t *mutex);

int __unmap(void *addr, size_t size);

int __unprotect(void *addr, size_t size);

int __wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

__END_DECLS

#endif // UTILS_H
