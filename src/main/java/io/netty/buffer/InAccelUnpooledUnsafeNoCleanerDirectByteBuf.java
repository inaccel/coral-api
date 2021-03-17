package io.netty.buffer;

import java.nio.ByteBuffer;

public class InAccelUnpooledUnsafeNoCleanerDirectByteBuf extends UnpooledUnsafeNoCleanerDirectByteBuf {

	public InAccelUnpooledUnsafeNoCleanerDirectByteBuf(ByteBufAllocator alloc, int initialCapacity, int maxCapacity) {
		super(alloc, initialCapacity, maxCapacity);
	}

	@Override
	protected ByteBuffer allocateDirect(int initialCapacity) {
		return super.allocateDirect(initialCapacity);
	}

	@Override
	protected void freeDirect(ByteBuffer buffer) {
		super.freeDirect(buffer);
	}

	@Override
	protected ByteBuffer reallocateDirect(ByteBuffer oldBuffer, int initialCapacity) {
		return super.reallocateDirect(oldBuffer, initialCapacity);
	}

}
