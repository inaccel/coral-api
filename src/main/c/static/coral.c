#include <dlfcn.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "coral.h"

#define PANIC(format, ...) ({ \
	fprintf(stderr, "panic: " format "\n", ## __VA_ARGS__); \
	fprintf(stderr, "at %s(%s:%d)\n", __FUNCTION__, __FILE__, __LINE__); \
	exit(EXIT_FAILURE); \
})

static struct {
	void *(*inaccel_alloc)(size_t size);
	void (*inaccel_free)(void *ptr);
	void *(*inaccel_realloc)(void *ptr, size_t size);
	int (*inaccel_request_arg_array)(inaccel_request request, size_t size, const void *value, unsigned index);
	int (*inaccel_request_arg_scalar)(inaccel_request request, size_t size, const void *value, unsigned index);
	inaccel_request (*inaccel_request_create)(const char *accelerator);
	int (*inaccel_request_fprint)(FILE *stream, const inaccel_request request);
	void (*inaccel_request_release)(inaccel_request request);
	int (*inaccel_request_snprint)(char *s, size_t n, const inaccel_request request);
	inaccel_response (*inaccel_response_create)(void);
	int (*inaccel_response_fprint)(FILE *stream, const inaccel_response response);
	void (*inaccel_response_release)(inaccel_response response);
	int (*inaccel_response_snprint)(char *s, size_t n, const inaccel_response response);
	int (*inaccel_response_wait)(inaccel_response response);
	int (*inaccel_response_wait_for)(inaccel_response response, time_t sec, long nsec);
	int (*inaccel_submit)(inaccel_request request, inaccel_response response);
} dl;

extern char _binary_libcoral_api_so_end;
extern char _binary_libcoral_api_so_start;

__attribute__ ((constructor (101)))
static void __init(void);

static void __init(void) {
	char *tmpdir = getenv("TMPDIR");
	if (!tmpdir) {
		tmpdir = "/tmp";
	}
	char filename[PATH_MAX];
	if (sprintf(filename, "%s/libXXXXXX.so", tmpdir) < 0) {
		PANIC("sprintf: %s", strerror(errno));
	}
	int fd = mkstemps(filename, strlen(".so"));
	if (fd == -1) {
		PANIC("mkstemps: %s", strerror(errno));
	}
	if (close(fd)) {
		PANIC("close: %s", strerror(errno));
	}

	FILE *stream = fopen(filename , "w");
	if (!stream) {
		PANIC("fopen: %s", strerror(errno));
	}
	size_t _binary_libcoral_api_so_size = &_binary_libcoral_api_so_end - &_binary_libcoral_api_so_start;
	if (fwrite(&_binary_libcoral_api_so_start, sizeof(char), _binary_libcoral_api_so_size, stream) != _binary_libcoral_api_so_size) {
		PANIC("fwrite: %s", strerror(errno));
	}
	if (fclose(stream)) {
		PANIC("fclose: %s", strerror(errno));
	}

	void *handle = dlopen(filename, RTLD_LAZY | RTLD_NODELETE);
	if (!handle) {
		PANIC("dlopen: %s", dlerror());
	}
	if (!(dl.inaccel_alloc = (void *(*)(size_t)) dlsym(handle, "inaccel_alloc"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_free = (void (*)(void *)) dlsym(handle, "inaccel_free"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_realloc = (void *(*)(void *, size_t)) dlsym(handle, "inaccel_realloc"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_arg_array = (int (*)(inaccel_request, size_t, const void *, unsigned)) dlsym(handle, "inaccel_request_arg_array"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_arg_scalar = (int (*)(inaccel_request, size_t, const void *, unsigned)) dlsym(handle, "inaccel_request_arg_scalar"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_create = (inaccel_request (*)(const char *)) dlsym(handle, "inaccel_request_create"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_fprint = (int (*)(FILE *, const inaccel_request)) dlsym(handle, "inaccel_request_fprint"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_release = (void (*)(inaccel_request)) dlsym(handle, "inaccel_request_release"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_request_snprint = (int (*)(char *, size_t, const inaccel_request)) dlsym(handle, "inaccel_request_snprint"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_create = (inaccel_response (*)(void)) dlsym(handle, "inaccel_response_create"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_fprint = (int (*)(FILE *, const inaccel_response)) dlsym(handle, "inaccel_response_fprint"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_release = (void (*)(inaccel_response)) dlsym(handle, "inaccel_response_release"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_snprint = (int (*)(char *, size_t, const inaccel_response)) dlsym(handle, "inaccel_response_snprint"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_wait = (int (*)(inaccel_response)) dlsym(handle, "inaccel_response_wait"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_response_wait_for = (int (*)(inaccel_response, time_t, long)) dlsym(handle, "inaccel_response_wait_for"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (!(dl.inaccel_submit = (int (*)(inaccel_request, inaccel_response)) dlsym(handle, "inaccel_submit"))) {
		PANIC("dlsym: %s", dlerror());
	}
	if (dlclose(handle)) {
		PANIC("dlclose: %s", dlerror());
	}

	if (unlink(filename)) {
		PANIC("unlink: %s", strerror(errno));
	}
}

void *inaccel_alloc(size_t size) {
	return dl.inaccel_alloc(size);
}

void inaccel_free(void *ptr) {
	dl.inaccel_free(ptr);
}

void *inaccel_realloc(void *ptr, size_t size) {
	return dl.inaccel_realloc(ptr, size);
}

int inaccel_request_arg_array(inaccel_request request, size_t size, const void *value, unsigned index) {
	return dl.inaccel_request_arg_array(request, size, value, index);
}

int inaccel_request_arg_scalar(inaccel_request request, size_t size, const void *value, unsigned index) {
	return dl.inaccel_request_arg_scalar(request, size, value, index);
}

inaccel_request inaccel_request_create(const char *accelerator) {
	return dl.inaccel_request_create(accelerator);
}

int inaccel_request_fprint(FILE *stream, const inaccel_request request) {
	return dl.inaccel_request_fprint(stream, request);
}

void inaccel_request_release(inaccel_request request) {
	dl.inaccel_request_release(request);
}

int inaccel_request_snprint(char *s, size_t n, const inaccel_request request) {
	return dl.inaccel_request_snprint(s, n, request);
}

inaccel_response inaccel_response_create(void) {
	return dl.inaccel_response_create();
}

int inaccel_response_fprint(FILE *stream, const inaccel_response response) {
	return dl.inaccel_response_fprint(stream, response);
}

void inaccel_response_release(inaccel_response response) {
	dl.inaccel_response_release(response);
}

int inaccel_response_snprint(char *s, size_t n, const inaccel_response response) {
	return dl.inaccel_response_snprint(s, n, response);
}

int inaccel_response_wait(inaccel_response response) {
	return dl.inaccel_response_wait(response);
}

int inaccel_response_wait_for(inaccel_response response, time_t sec, long nsec) {
	return dl.inaccel_response_wait_for(response, sec, nsec);
}

int inaccel_submit(inaccel_request request, inaccel_response response) {
	return dl.inaccel_submit(request, response);
}
