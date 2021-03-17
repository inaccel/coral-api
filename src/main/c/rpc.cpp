#include <atomic>
#include <cerrno>
#include <condition_variable>
#include <grpc++/create_channel.h>
#include <stdint.h>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#include "cube.h"
#include "debug.h"
#include "rpc.h"
#include "self.h"
#include "utils.h"

#include "coral.grpc.pb.h"

static pthread_mutex_t it = PTHREAD_MUTEX_INITIALIZER;

#define LOCK pthread_mutex_lock(&it)
#define UNLOCK pthread_mutex_unlock(&it)

static grpc::CompletionQueue *cq;

static std::atomic<unsigned long> stamp;

static char target[PATH_MAX];

struct _inaccel_request {
	inaccel::Request grpc;
};

struct _inaccel_response {
	inaccel::Request request;

	inaccel::Response grpc;

	grpc::ClientContext context;
	grpc::Status status;

	pthread_cond_t cond;
	pthread_mutex_t mutex;

	bool cancelled;
	bool done;
};

static void __init();

static void *__waiter(void *arg);

__attribute__ ((constructor))
static void __init() {
	char *coral = getenv("INACCEL_CORAL");
	if (!coral) {
		if (sprintf(target, "unix:" INACCEL_RUN "/coral.sock") < 0) {
			PERROR("__init: sprintf");
		}
	} else {
		if (sprintf(target, "unix:" INACCEL_RUN "/%s.sock", coral) < 0) {
			PERROR("__init: sprintf");
		}
	}
}

static void *__waiter(void *arg) {
	while (true) {
		try {
			inaccel_response response;
			bool ok;
			if (cq->Next((void **) &response, &ok) && ok) {
				__lock(&response->mutex);

				inaccel::Arguments *__arguments = response->request.mutable_arguments();

				for (int index = 0; index < __arguments->argument_size(); index++) {
					inaccel::Argument *__argument = __arguments->mutable_argument(index);

					if (__argument->has_array()) {
						inaccel::Array *__array = __argument->mutable_array();

						if (__detach((slice_t *) __array->context())) {
							PERROR("__waiter: __detach");
						}
					}
				}

				if (response->cancelled) {
					__unlock(&response->mutex);

					if (__destroy_mutex(&response->mutex)) {
						PERROR("__waiter: __destroy_mutex");
					}

					if (__destroy_cond(&response->cond)) {
						PERROR("__waiter: __destroy_cond");
					}

					delete response;
				} else {
					response->done = true;

					__broadcast(&response->cond);

					__unlock(&response->mutex);
				}
			}
		} catch (...) {}
	}

	return NULL;
}

static inaccel_request __alloc_request() {
	try {
		return new _inaccel_request();
	} catch (...) {
		return NULL;
	}
}

static void __free_request(inaccel_request request) {
	try {
		delete request;
	} catch (...) {}
}

__attribute__ ((visibility ("default")))
int inaccel_request_arg_array(inaccel_request request, size_t size, const void *value, unsigned index) {
	if (!request || !size) {
		errno = EINVAL;
		return -1;
	}

	slice_t *slice = __slice(value, size);
	if (!slice) {
		return -1;
	}

	if (slice->cube->pid != __process()) {
		errno = EACCES;
		return -1;
	}

	try {
		inaccel::Arguments *__arguments = request->grpc.mutable_arguments();

		while ((unsigned) __arguments->argument_size() <= index) {
			__arguments->add_argument();
		}

		inaccel::Argument *__argument = __arguments->mutable_argument(index);

		inaccel::Array *__array = __argument->mutable_array();

		__array->set_context(__from_ptr(slice));

		return 0;
	} catch (...) {
		return -1;
	}
}

__attribute__ ((visibility ("default")))
int inaccel_request_arg_scalar(inaccel_request request, size_t size, const void *value, unsigned index) {
	if (!request || !size) {
		errno = EINVAL;
		return -1;
	}

	try {
		inaccel::Arguments *__arguments = request->grpc.mutable_arguments();

		while ((unsigned) __arguments->argument_size() <= index) {
			__arguments->add_argument();
		}

		inaccel::Argument *__argument = __arguments->mutable_argument(index);

		inaccel::Scalar *__scalar = __argument->mutable_scalar();

		__scalar->set_bytes(value, size);

		return 0;
	} catch (...) {
		return -1;
	}
}

__attribute__ ((visibility ("default")))
inaccel_request inaccel_request_create(const char *accelerator) {
	if (!accelerator) {
		errno = EINVAL;
		return NULL;
	}

	inaccel_request request = __alloc_request();
	if (!request) {
		return NULL;
	}

	try {
		request->grpc.set_accelerator(accelerator);
	} catch (...) {
		__free_request(request);

		return NULL;
	}

	return request;
}

int __str(std::stringstream &string, const inaccel_request request) {
	string << "---" << std::endl << request->grpc.accelerator() << ":";

	inaccel::Arguments __arguments = request->grpc.arguments();

	for (int index = 0; index < __arguments.argument_size(); index++) {
		string << std::endl;

		inaccel::Argument __argument = __arguments.argument(index);

		if (__argument.has_array()) {
			inaccel::Array __array = __argument.array();

			string << "- array:";

			slice_t *slice = (slice_t *) __array.context();

			if (slice->cube->pid != __process()) {
				errno = EACCES;
				return -1;
			}

			string << std::endl << "    id: " << slice->cube->id;
			string << std::endl << "    offset: " << slice->offset;
			string << std::endl << "    size: " << slice->size;
			string << std::endl << "    version: " << slice->version;
		}

		if (__argument.has_scalar()) {
			inaccel::Scalar scalar = __argument.scalar();

			string << "- scalar:";
			string << std::endl << "    bytes: 0x";
			const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
			for (char byte : scalar.bytes()) {
				string << hex[(byte & 0xF0) >> 4];
				string << hex[(byte & 0x0F) >> 0];
			}
		}
	}

	return 0;
}

__attribute__ ((visibility ("default")))
int inaccel_request_fprint(FILE *stream, const inaccel_request request) {
	if (!request) {
		errno = EINVAL;
		return -1;
	}

	try {
		std::stringstream string;

		int error = __str(string, request);
		if (error) {
			return error;
		}

		return fprintf(stream, string.str().c_str(), string.str().size());
	} catch (...) {
		return -1;
	}
}

__attribute__ ((visibility ("default")))
void inaccel_request_release(inaccel_request request) {
	if (!request) {
		return;
	}

	__free_request(request);
}

__attribute__ ((visibility ("default")))
int inaccel_request_snprint(char *s, size_t n, const inaccel_request request) {
	if (!request) {
		errno = EINVAL;
		return -1;
	}

	try {
		std::stringstream string;

		int error = __str(string, request);
		if (error) {
			return error;
		}

		return snprintf(s, n, string.str().c_str(), string.str().size());
	} catch (...) {
		return -1;
	}
}

static inaccel_response __alloc_response() {
	try {
		return new _inaccel_response();
	} catch (...) {
		return NULL;
	}
}

static void __free_response(inaccel_response response) {
	try {
		delete response;
	} catch (...) {}
}

__attribute__ ((visibility ("default")))
inaccel_response inaccel_response_create() {
	inaccel_response response = __alloc_response();
	if (!response) {
		return NULL;
	}

	if (__init_cond(&response->cond, PROCESS_SHARED)) {
		int errsv = errno;

		PERROR("inaccel_response_create: __init_cond");

		__free_response(response);

		errno = errsv;
		return NULL;
	}

	if (__init_mutex(&response->mutex, PROCESS_SHARED)) {
		int errsv = errno;

		PERROR("inaccel_response_create: __init_mutex");

		__free_response(response);

		errno = errsv;
		return NULL;
	}

	return response;
}

__attribute__ ((visibility ("default")))
void inaccel_response_release(inaccel_response response) {
	if (!response) {
		return;
	}

	__lock(&response->mutex);

	if (response->done) {
		__unlock(&response->mutex);

		if (__destroy_mutex(&response->mutex)) {
			PERROR("inaccel_response_release: __destroy_mutex");
		}

		if (__destroy_cond(&response->cond)) {
			PERROR("inaccel_response_release: __destroy_cond");
		}

		__free_response(response);
	} else {
		response->cancelled = true;

		__broadcast(&response->cond);

		__unlock(&response->mutex);
	}
}

__attribute__ ((visibility ("default")))
int inaccel_response_wait(inaccel_response response) {
	if (!response) {
		errno = EINVAL;
		return -1;
	}

	__lock(&response->mutex);

	while (true) {
		if (response->cancelled || response->done) {
			break;
		}

		if (__wait(&response->cond, &response->mutex)) {
			int errsv = errno;

			__unlock(&response->mutex);

			errno = errsv;
			return -1;
		}
	}

	__unlock(&response->mutex);

	return response->status.error_code();
}

__attribute__ ((visibility ("default")))
int inaccel_response_wait_for(inaccel_response response, time_t sec, long nsec) {
	if (!response) {
		errno = EINVAL;
		return -1;
	}

	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME, &abstime);

	struct timespec reltime;
	reltime.tv_sec = sec;
	reltime.tv_nsec = nsec;

	abstime.tv_sec += reltime.tv_sec;
	abstime.tv_nsec += reltime.tv_nsec;
	if (abstime.tv_nsec >= 1000000000L) {
		abstime.tv_sec++;
		abstime.tv_nsec -= 1000000000L;
	}

	__lock(&response->mutex);

	while (true) {
		if (response->cancelled || response->done) {
			break;
		}

		if (__timedwait(&response->cond, &response->mutex, &abstime)) {
			int errsv = errno;

			__unlock(&response->mutex);

			errno = errsv;
			return -1;
		}
	}

	__unlock(&response->mutex);

	return response->status.error_code();
}

__attribute__ ((visibility ("default")))
int inaccel_submit(inaccel_request request, inaccel_response response) {
	if (!request || !response) {
		errno = EINVAL;
		return -1;
	}

	try {
		inaccel::Arguments *__arguments = request->grpc.mutable_arguments();

		for (int index = 0; index < __arguments->argument_size(); index++) {
			inaccel::Argument *__argument = __arguments->mutable_argument(index);

			if (__argument->has_array()) {
				inaccel::Array *__array = __argument->mutable_array();

				slice_t *slice = (slice_t *) __array->context();

				if (slice->cube->pid != __process()) {
					errno = EACCES;
					return -1;
				}

				__array->set_id(slice->cube->id);
				__array->set_offset(slice->offset);
				__array->set_size(slice->size);
				__array->set_version(slice->version);

				if (__attach(slice)) {
					return -1;
				}
			}
		}

		response->request = request->grpc;

		inaccel::Metadata *__metadata = request->grpc.mutable_metadata();

		__metadata->set_self(self);
		__metadata->set_stamp(stamp++);

		LOCK;
		if (!cq) {
			cq = new grpc::CompletionQueue;

			char *extra_waiters = getenv("INACCEL_EXTRA_WAITERS");
			if (extra_waiters) {
				for (int index = 0; index < atoi(extra_waiters); index++) {
					if (__daemon(&__waiter, NULL)) {
						int errsv = errno;

						UNLOCK;

						errno = errsv;
						return -1;
					}
				}
			}
			if (__daemon(&__waiter, NULL)) {
				int errsv = errno;

				UNLOCK;

				errno = errsv;
				return -1;
			}
		}
		UNLOCK;

		inaccel::Coral::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()))
			->AsyncServe(&response->context, request->grpc, cq)
			->Finish(&response->grpc, &response->status, (void *) response);

		return 0;
	} catch(...) {
		return -1;
	}
}
