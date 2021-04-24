package io.netty.buffer;

import com.sun.jna.Native;
import com.sun.jna.Pointer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public abstract class InAccelUnpooledDirectByteBuf extends UnpooledDirectByteBuf {

	public InAccelUnpooledDirectByteBuf(ByteBufAllocator alloc, int initialCapacity, int maxCapacity) {
		super(alloc, initialCapacity, maxCapacity);
	}

	@Override
	protected abstract ByteBuffer allocateDirect(int initialCapacity);

	@Override
	public ByteBuf capacity(int newCapacity) {
		checkNewCapacity(newCapacity);

		int oldCapacity = capacity();
		if (newCapacity == oldCapacity) {
			return this;
		}

		trimIndicesToCapacity(newCapacity);
		setByteBuffer(reallocateDirect(buffer, newCapacity), false);
		return this;
	}

	public static ByteBuffer directBuffer(long memoryAddress, int size) {
		return new Pointer(memoryAddress).getByteBuffer(0, size).order(ByteOrder.BIG_ENDIAN);
	}

	public static long directBufferAddress(ByteBuffer buffer) {
		return Pointer.nativeValue(Native.getDirectBufferPointer(buffer));
	}

	@Override
	protected abstract void freeDirect(ByteBuffer buffer);

	@Override
	public boolean hasMemoryAddress() {
		return true;
	}

	@Override
	public long memoryAddress() {
		ensureAccessible();
		return directBufferAddress(buffer);
	}

	protected abstract ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity);

}
