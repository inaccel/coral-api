package com.inaccel.coral;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

import java.nio.charset.StandardCharsets;
import java.util.concurrent.Callable;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.atomic.DoubleAccumulator;
import java.util.concurrent.atomic.DoubleAdder;
import java.util.concurrent.atomic.LongAccumulator;
import java.util.concurrent.atomic.LongAdder;

public final class InAccel {

	public static class Request {

		final long c;

		private int index = 0;

		public Request(String accelerator) throws RuntimeException {
			c = Jni.inaccel_request_create(ByteBufAllocator.DEFAULT.directBuffer().writeBytes(accelerator.getBytes()).writeByte(0).memoryAddress());
			if (c == 0) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}
		}

		public <T extends Number> Request arg(T value) throws IllegalArgumentException, RuntimeException {
			return arg(value, index++);
		}

		public <T extends Number> Request arg(T value, int index) throws IllegalArgumentException, RuntimeException {
			if (value instanceof Byte) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Byte.BYTES).writeByte(value.byteValue()), index);
			} else if (value instanceof Double || value instanceof DoubleAccumulator || value instanceof DoubleAdder) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Double.BYTES).writeDouble(value.doubleValue()), index);
			} else if (value instanceof Float) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Float.BYTES).writeFloat(value.floatValue()), index);
			} else if (value instanceof AtomicInteger || value instanceof Integer) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Integer.BYTES).writeInt(value.intValue()), index);
			} else if (value instanceof AtomicLong || value instanceof Long || value instanceof LongAccumulator || value instanceof LongAdder) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Long.BYTES).writeLong(value.longValue()), index);
			} else if (value instanceof Short) {
				return arg(ByteBufAllocator.DEFAULT.directBuffer(Short.BYTES).writeShort(value.shortValue()), index);
			} else {
				throw new IllegalArgumentException();
			}
		}

		public Request arg(ByteBuf value) throws RuntimeException {
			return arg(value, index++);
		}

		public Request arg(ByteBuf value, int index) throws RuntimeException {
			if (!value.isDirect()) {
				value = ByteBufAllocator.DEFAULT.directBuffer(value.capacity()).writeBytes(value);
			}

			if (value.alloc() instanceof InAccelByteBufAllocator) {
				int error = Jni.inaccel_request_arg_array(c, value.capacity(), value.memoryAddress(), index);
				if (error != 0) {
					throw new RuntimeException(C.library.strerror(Jni.errno()));
				}
			} else {
				int error = Jni.inaccel_request_arg_scalar(c, value.capacity(), value.memoryAddress(), index);
				if (error != 0) {
					throw new RuntimeException(C.library.strerror(Jni.errno()));
				}
			}

			return this;
		}

		@Override
		protected void finalize() throws Throwable {
			Jni.inaccel_request_release(c);
		}

		@Override
		public String toString() throws RuntimeException {
			int n = Jni.inaccel_request_snprint(Jni.NULL, 0, c);
			if (n < 0) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			ByteBuf s = ByteBufAllocator.DEFAULT.directBuffer(n + 1);
			if (Jni.inaccel_request_snprint(s.memoryAddress(), s.capacity(), c) != n) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			return s.toString(0, s.capacity(), StandardCharsets.UTF_8);
		}

	}

	public static Future<Void> submit(Request request) throws RuntimeException {
		final long cresponse = Jni.inaccel_response_create();
		if (cresponse == 0) {
			throw new RuntimeException(C.library.strerror(Jni.errno()));
		}

		int error = Jni.inaccel_submit(request.c, cresponse);
		if (error != 0) {
			int errsv = Jni.errno();

			Jni.inaccel_response_release(cresponse);

			throw new RuntimeException(C.library.strerror(errsv));
		}

		final ExecutorService service = Executors.newSingleThreadExecutor(new ThreadFactory() {

			public Thread newThread(Runnable r) {
				Thread t = Executors.defaultThreadFactory().newThread(r);
				t.setDaemon(true);
				return t;
			}

		});

		return service.submit(new Callable<Void>() {

			public Void call() throws Exception {
				int error = Jni.inaccel_response_wait(cresponse);
				if (error == -1) {
					int errsv = Jni.errno();

					Jni.inaccel_response_release(cresponse);

					service.shutdown();

					throw new RuntimeException(C.library.strerror(errsv));
				} else if (error != 0) {
					int n = Jni.inaccel_response_snprint(Jni.NULL, 0, cresponse);
					if (n < 0) {
						int errsv = Jni.errno();

						Jni.inaccel_response_release(cresponse);

						service.shutdown();

						throw new RuntimeException(C.library.strerror(errsv));
					}

					ByteBuf s = ByteBufAllocator.DEFAULT.directBuffer(n + 1);
					if (Jni.inaccel_response_snprint(s.memoryAddress(), s.capacity(), cresponse) != n) {
						int errsv = Jni.errno();

						Jni.inaccel_response_release(cresponse);

						service.shutdown();

						throw new RuntimeException(C.library.strerror(errsv));
					}

					Jni.inaccel_response_release(cresponse);

					service.shutdown();

					throw new Exception(s.toString(0, s.capacity(), StandardCharsets.UTF_8));
				}

				Jni.inaccel_response_release(cresponse);

				service.shutdown();

				return null;
			}

		});
	}

}
