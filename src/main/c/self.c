#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "self.h"
#include "utils.h"

temp_t self;

__attribute__ ((constructor (102)))
static void __atfork();

__attribute__ ((constructor (102)))
static void __init();

static int __mkself(char *path);

static void __atfork() {
	SYSLOG(pthread_atfork(NULL, NULL, __init));
}

int __close(int fd) {
	return close(fd);
}

static void __init() {
	char path[PATH_MAX];
	if (!SYSLOG(__mkself(path))) {
		pid_t pid = __process();
		if (!SYSLOG_NEGATIVE(__fork())) {
			if (SYSLOG(__session(pid))) {
				__exit(1);
			}

			do {
				__sleep(1);
			} while (!__srch(pid));

			if (SYSLOG(__remove(path))) {
				__exit(1);
			}

			__exit(0);
		}
	}
}

int __link_open(temp_t id) {
	char path[PATH_MAX];
	if (sprintf(path, INACCEL_SHM "/%.6s/XXXXXX", INACCEL, self) < 0) {
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
	if (sprintf(path, INACCEL_SHM "/XXXXXX", INACCEL) < 0) {
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
	if (sprintf(path, INACCEL_SHM "/%.6s/%.6s", INACCEL, self, id) < 0) {
		return -1;
	}
	return open(path, O_RDWR, S_IRUSR | S_IWUSR);
}

int __unlink(const temp_t id) {
	char path[PATH_MAX];
	if (sprintf(path, INACCEL_SHM "/%.6s/%.6s", INACCEL, self, id) < 0) {
		return -1;
	}
	return unlink(path);
}
