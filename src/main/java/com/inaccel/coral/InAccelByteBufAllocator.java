package com.inaccel.coral;

import io.netty.buffer.AbstractByteBufAllocator;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;

public final class InAccelByteBufAllocator extends AbstractByteBufAllocator {

	public static final ByteBufAllocator DEFAULT = new InAccelByteBufAllocator();

	private final boolean disableLeakDetector;

	public InAccelByteBufAllocator() {
		this(false);
	}

	public InAccelByteBufAllocator(boolean disableLeakDetector) {
		super(true);

		this.disableLeakDetector = disableLeakDetector;
	}

	public boolean isDirectBufferPooled() {
		return false;
	}

	protected ByteBuf newDirectBuffer(int initialCapacity, int maxCapacity) {
		final ByteBuf buf = new InAccelByteBuf(this, initialCapacity, maxCapacity);

		return disableLeakDetector ? buf : toLeakAwareBuffer(buf);
	}

	protected ByteBuf newHeapBuffer(int initialCapacity, int maxCapacity) {
		throw new UnsupportedOperationException();
	}

}
