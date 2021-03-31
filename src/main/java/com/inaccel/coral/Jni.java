package com.inaccel.coral;

final class Jni {

	static {
		Library.load("coral-api", Jni.class);
	}

	static long NULL;

	static native int errno();

	static native long inaccel_alloc(long size);

	static native void inaccel_free(long ptr);

	static native long inaccel_realloc(long ptr, long size);

	static native int inaccel_request_arg_array(long request, long size, long value, int index);

	static native int inaccel_request_arg_scalar(long request, long size, long value, int index);

	static native long inaccel_request_create(long accelerator);

	static native int inaccel_request_snprint(long s, long n, long request);

	static native void inaccel_request_release(long request);

	static native long inaccel_response_create();

	static native void inaccel_response_release(long response);

	static native int inaccel_response_snprint(long s, long n, long response);

	static native int inaccel_response_wait(long response);

	static native int inaccel_submit(long request, long response);

}
