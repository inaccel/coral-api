import com.inaccel.coral.*;

import io.netty.buffer.ByteBuf;

import java.util.concurrent.Future;

public class Example {

	public static void main(String[] args) throws Exception {
		int size = 1024;

		ByteBuf a = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);
		ByteBuf b = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);
		ByteBuf c = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);

		// ...

		InAccel.Request request = new InAccel.Request("vector.addition");

		request.arg(a).arg(b).arg(c).arg(size);

		System.out.println(request);

		Future<Void> response = InAccel.submit(request);

		response.get();

		// ...

		c.release();
		b.release();
		a.release();
	}

}
