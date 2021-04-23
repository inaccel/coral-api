package com.inaccel.coral;

import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.InAccelUnpooledDirectByteBuf;

import java.nio.ByteBuffer;

final class InAccelByteBuf extends InAccelUnpooledDirectByteBuf {

	InAccelByteBuf(ByteBufAllocator alloc, int initialCapacity, int maxCapacity) {
		super(alloc, initialCapacity, maxCapacity);
	}

	@Override
	protected ByteBuffer allocateDirect(int initialCapacity) {
		long memoryAddress = Jni.inaccel_alloc(initialCapacity);
		if (memoryAddress == Jni.NULL) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		return directBuffer(memoryAddress, initialCapacity);
	}

	@Override
	protected void freeDirect(ByteBuffer buffer) {
		Jni.inaccel_free(directBufferAddress(buffer));
	}

	@Override
	protected ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity) {
		long memoryAddress = Jni.inaccel_realloc(directBufferAddress(oldBuffer), initialCapacity);
		if (memoryAddress == Jni.NULL) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		return directBuffer(memoryAddress, initialCapacity);
	}

}
