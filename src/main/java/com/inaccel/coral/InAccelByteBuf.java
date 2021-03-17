package com.inaccel.coral;

import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.InAccelUnpooledUnsafeNoCleanerDirectByteBuf;
import io.netty.util.internal.PlatformDependent;

import java.nio.ByteBuffer;

final class InAccelByteBuf extends InAccelUnpooledUnsafeNoCleanerDirectByteBuf {

	InAccelByteBuf(ByteBufAllocator alloc, int initialCapacity, int maxCapacity) {
		super(alloc, initialCapacity, maxCapacity);
	}

	@Override
	protected ByteBuffer allocateDirect(int initialCapacity) {
		long memoryAddress = Jni.inaccel_alloc(initialCapacity);
		if (memoryAddress == 0) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		return PlatformDependent.directBuffer(memoryAddress, initialCapacity);
	}

	@Override
	protected void freeDirect(ByteBuffer buffer) {
		Jni.inaccel_free(PlatformDependent.directBufferAddress(buffer));
	}

	@Override
	protected ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity) {
		long memoryAddress = Jni.inaccel_realloc(PlatformDependent.directBufferAddress(oldBuffer), initialCapacity);
		if (memoryAddress == 0) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		return PlatformDependent.directBuffer(memoryAddress, initialCapacity);
	}

}
