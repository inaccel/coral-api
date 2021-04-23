package com.inaccel.coral;

import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.InAccelUnpooledDirectByteBuf;
import io.netty.util.internal.ObjectCleaner;

import java.nio.ByteBuffer;
import java.util.Objects;

final class InAccelByteBuf extends InAccelUnpooledDirectByteBuf {

	private static class Finalize implements Runnable {

		private long ptr;

		private void set(long ptr) {
			this.ptr = ptr;
		}

		public void run() {
			Jni.inaccel_free(ptr);
		}

	}

	private Finalize finalize;

	private void finalize(long ptr) {
		if (Objects.isNull(finalize)) {
			finalize = new Finalize();

			ObjectCleaner.register(this, finalize);
		}

		finalize.set(ptr);
	}

	InAccelByteBuf(ByteBufAllocator alloc, int initialCapacity, int maxCapacity) {
		super(alloc, initialCapacity, maxCapacity);
	}

	@Override
	protected ByteBuffer allocateDirect(int initialCapacity) {
		long memoryAddress = Jni.inaccel_alloc(initialCapacity);
		if (memoryAddress == Jni.NULL) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		finalize(memoryAddress);
		return directBuffer(memoryAddress, initialCapacity);
	}

	@Override
	protected void freeDirect(ByteBuffer buffer) {
		Jni.inaccel_free(directBufferAddress(buffer));
		finalize(Jni.NULL);
	}

	@Override
	protected ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity) {
		long memoryAddress = Jni.inaccel_realloc(directBufferAddress(oldBuffer), initialCapacity);
		if (memoryAddress == Jni.NULL) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		finalize(memoryAddress);
		return directBuffer(memoryAddress, initialCapacity);
	}

}
