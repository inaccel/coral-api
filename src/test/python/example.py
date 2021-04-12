import inaccel.coral as inaccel
import numpy as np

size = np.int32(1024)

with inaccel.allocator:
    a = np.ndarray(size, dtype=np.float32)
    b = np.ndarray(size, dtype=np.float32)
    c = np.ndarray(size, dtype=np.float32)

request = inaccel.request('vector.addition')

request.arg(a).arg(b).arg(c).arg(size)

print(request)

response = inaccel.submit(request)

response.result()
