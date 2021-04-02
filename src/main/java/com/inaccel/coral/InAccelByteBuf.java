package com.inaccel.coral;

import com.sun.jna.Native;
import com.sun.jna.Pointer;

import io.netty.buffer.ByteBufAllocator;
import io.netty.buffer.InAccelUnpooledUnsafeNoCleanerDirectByteBuf;

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
		return directBuffer(memoryAddress, initialCapacity);
	}

	private static ByteBuffer directBuffer(long memoryAddress, int size) {
		return new Pointer(memoryAddress).getByteBuffer(0, size);
	}

	private static long directBufferAddress(ByteBuffer buffer) {
		return Pointer.nativeValue(Native.getDirectBufferPointer(buffer));
	}

	@Override
	protected void freeDirect(ByteBuffer buffer) {
		Jni.inaccel_free(directBufferAddress(buffer));
	}

	@Override
	protected ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity) {
		long memoryAddress = Jni.inaccel_realloc(directBufferAddress(oldBuffer), initialCapacity);
		if (memoryAddress == 0) {
			throw new OutOfMemoryError(C.library.strerror(Jni.errno()));
		}
		return directBuffer(memoryAddress, initialCapacity);
	}

}
