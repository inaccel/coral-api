package com.inaccel.coral;

import io.netty.buffer.ByteBuf;
import io.netty.util.internal.ObjectCleaner;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
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

	public static final class Request {

		private static class Finalize implements Runnable {

			private final long request;

			private Finalize(long request) {
				this.request = request;
			}

			public void run() {
				Jni.inaccel_request_release(request);
			}

		}

		final long c;

		private int index = 0;

		public Request(String accelerator) throws RuntimeException {
			byte[] bytes = accelerator.getBytes(StandardCharsets.UTF_8);

			c = Jni.inaccel_request_create(InAccelByteBuf.directBufferAddress(ByteBuffer.allocateDirect(bytes.length + 1).put(bytes).put((byte) 0)));
			if (c == 0) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			ObjectCleaner.register(this, new Finalize(c));
		}

		public <T extends Number> Request arg(T value) throws IllegalArgumentException, RuntimeException {
			return arg(value, index++);
		}

		public <T extends Number> Request arg(T value, int index) throws IllegalArgumentException, RuntimeException {
			if (value instanceof Byte) {
				return arg(ByteBuffer.allocateDirect(Byte.BYTES).put(value.byteValue()), index);
			} else if (value instanceof Double || value instanceof DoubleAccumulator || value instanceof DoubleAdder) {
				return arg(ByteBuffer.allocateDirect(Double.BYTES).order(ByteOrder.nativeOrder()).putDouble(value.doubleValue()), index);
			} else if (value instanceof Float) {
				return arg(ByteBuffer.allocateDirect(Float.BYTES).order(ByteOrder.nativeOrder()).putFloat(value.floatValue()), index);
			} else if (value instanceof AtomicInteger || value instanceof Integer) {
				return arg(ByteBuffer.allocateDirect(Integer.BYTES).order(ByteOrder.nativeOrder()).putInt(value.intValue()), index);
			} else if (value instanceof AtomicLong || value instanceof Long || value instanceof LongAccumulator || value instanceof LongAdder) {
				return arg(ByteBuffer.allocateDirect(Long.BYTES).order(ByteOrder.nativeOrder()).putLong(value.longValue()), index);
			} else if (value instanceof Short) {
				return arg(ByteBuffer.allocateDirect(Short.BYTES).order(ByteOrder.nativeOrder()).putShort(value.shortValue()), index);
			} else {
				throw new IllegalArgumentException();
			}
		}

		public Request arg(ByteBuffer value) throws RuntimeException {
			return arg(value, index++);
		}

		public Request arg(ByteBuffer value, int index) throws RuntimeException {
			int error = Jni.inaccel_request_arg_scalar(c, value.capacity(), InAccelByteBuf.directBufferAddress(value), index);
			if (error != 0) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			return this;
		}

		public Request arg(ByteBuf value) throws RuntimeException {
			return arg(value, index++);
		}

		public Request arg(ByteBuf value, int index) throws RuntimeException {
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
		public String toString() throws RuntimeException {
			int n = Jni.inaccel_request_snprint(Jni.NULL, 0, c);
			if (n < 0) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			ByteBuffer s = ByteBuffer.allocateDirect(n + 1);
			if (Jni.inaccel_request_snprint(InAccelByteBuf.directBufferAddress(s), s.capacity(), c) != n) {
				throw new RuntimeException(C.library.strerror(Jni.errno()));
			}

			return StandardCharsets.UTF_8.decode(s).toString();
		}

	}

	public static Future<Void> submit(Request request) throws RuntimeException {
		final long cresponse = Jni.inaccel_response_create();
		if (cresponse == Jni.NULL) {
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

					ByteBuffer s = ByteBuffer.allocateDirect(n + 1);
					if (Jni.inaccel_response_snprint(InAccelByteBuf.directBufferAddress(s), s.capacity(), cresponse) != n) {
						int errsv = Jni.errno();

						Jni.inaccel_response_release(cresponse);

						service.shutdown();

						throw new RuntimeException(C.library.strerror(errsv));
					}

					Jni.inaccel_response_release(cresponse);

					service.shutdown();

					throw new Exception(StandardCharsets.UTF_8.decode(s).toString());
				}

				Jni.inaccel_response_release(cresponse);

				service.shutdown();

				return null;
			}

		});
	}

}
