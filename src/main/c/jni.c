#include <errno.h>

#include "coral.h"

static __thread int errsv;

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_errno(void *_, void *__) {
	return errsv;
}

__attribute__ ((visibility ("default")))
long long Java_com_inaccel_coral_Jni_inaccel_1alloc(void *_, void *__, long long size) {
	long long tmp = (long long) inaccel_alloc((size_t) size);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
void Java_com_inaccel_coral_Jni_inaccel_1free(void *_, void *__, long long ptr) {
	inaccel_free((void *) ptr);
}

__attribute__ ((visibility ("default")))
long long Java_com_inaccel_coral_Jni_inaccel_1realloc(void *_, void *__, long long ptr, long long size) {
	long long tmp = (long long) inaccel_realloc((void *) ptr, (size_t) size);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_inaccel_1request_1arg_1array(void *_, void *__, long long request, long long size, long long value, int index) {
	int tmp = inaccel_request_arg_array((inaccel_request) request, (size_t) size, (const void *) value, (unsigned) index);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_inaccel_1request_1arg_1scalar(void *_, void *__, long long request, long long size, long long value, int index) {
	int tmp = inaccel_request_arg_scalar((inaccel_request) request, (size_t) size, (const void *) value, (unsigned) index);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
long long Java_com_inaccel_coral_Jni_inaccel_1request_1create(void *_, void *__, long long accelerator) {
	long long tmp = (long long) inaccel_request_create((const char *) accelerator);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
void Java_com_inaccel_coral_Jni_inaccel_1request_1release(void *_, void *__, long long request) {
	inaccel_request_release((inaccel_request) request);
}

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_inaccel_1request_1snprint(void *_, void *__, long long s, long long n, long long request) {
	int tmp = inaccel_request_snprint((char *) s, (size_t) n, (const inaccel_request) request);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
long long Java_com_inaccel_coral_Jni_inaccel_1response_1create(void *_, void *__) {
 	long long tmp = (long long) inaccel_response_create();
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
void Java_com_inaccel_coral_Jni_inaccel_1response_1release(void *_, void *__, long long response) {
	inaccel_response_release((inaccel_response) response);
}

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_inaccel_1response_1wait(void *_, void *__, long long response) {
	int tmp = inaccel_response_wait((inaccel_response) response);
	errsv = errno;
	return tmp;
}

__attribute__ ((visibility ("default")))
int Java_com_inaccel_coral_Jni_inaccel_1submit(void *_, void *__, long long request, long long response) {
	int tmp = inaccel_submit((inaccel_request) request, (inaccel_response) response);
	errsv = errno;
	return tmp;
}
