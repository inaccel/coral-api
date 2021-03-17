#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "self.h"
#include "utils.h"

temp_t self;

static void __atfork();

static void __init();

static int __mkself(char *path);

__attribute__ ((constructor))
static void __atfork() {
	if (pthread_atfork(NULL, NULL, __init)) {
		PERROR("__atfork: pthread_atfork");
	}
}

int __close(int fd) {
	return close(fd);
}

__attribute__ ((constructor))
static void __init() {
	char path[PATH_MAX];
	if (__mkself(path)) {
		PERROR("__init: __mkself");

		return;
	}

	pid_t pid = __process();
	switch (__fork()) {
	case -1:
		PERROR("__init: __fork");

		return;
	case 0:
		if (__session(pid)) {
			int errsv = errno;

			PERROR("__init: __session");

			__exit(errsv);
		}

		do {
			__sleep(1);
		} while (!__srch(pid));

		__exit(__remove(path));
	}
}

int __link_open(temp_t id) {
	char path[PATH_MAX];
	if (sprintf(path, INACCEL_SHM "/%.6s/XXXXXX", self) < 0) {
		return -1;
	}
	int fd = mkstemp(path);
	if (fd == -1) {
		return -1;
	}
	strcpy(id, path + strlen(path) - 6);
	return fd;
}

static int __mkself(char *path) {
	if (sprintf(path, INACCEL_SHM "/XXXXXX") < 0) {
		return -1;
	}
	if (!mkdtemp(path)) {
		return -1;
	}
	strcpy(self, path + strlen(path) - 6);
	return 0;
}

int __reopen(const temp_t id) {
	char path[PATH_MAX];
	if (sprintf(path, INACCEL_SHM "/%.6s/%.6s", self, id) < 0) {
		return -1;
	}
	return open(path, O_RDWR, S_IRUSR | S_IWUSR);
}

int __unlink(const temp_t id) {
	char path[PATH_MAX];
	if (sprintf(path, INACCEL_SHM "/%.6s/%.6s", self, id) < 0) {
		return -1;
	}
	return unlink(path);
}
