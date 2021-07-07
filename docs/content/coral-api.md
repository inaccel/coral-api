# Introduction to Coral

This page introduces you to Coral and its Application Programming Interface
(API). If you’re new to Coral concepts, continue reading! If you just want to
dive in and see Coral API in action first, select a language and try its
[Quick start](#quick-start).

## Overview

In Coral, a client application can directly call an accelerator on a local
hardware resource as if it were a common software function, making it easier for
you to accelerate distributed applications and services. As in many RPC systems,
Coral is based around the idea of defining a platform, specifying the
accelerators that can be called remotely with their arguments and configuration
parameters. On the server side, Coral dynamically implements an interface and
runs a server to handle and respond to client calls. On the client side, the
application just requests the accelerators available at the server. Coral relies
on shared memory namespaces and uses gRPC as its underlying message interchange
system.

![Architecture](images/architecture.svg)

Coral clients and servers can run and talk to each other in a variety of
environments - from bare-metal Linux servers to containers inside a Kubernetes
cluster - and applications can be written in any of Coral API’s supported
languages. So, for example, you can easily use the same accelerator with a Coral
client in C++ or Python.

# Core concepts, architecture and lifecycle

Coral is a fast and general-purpose accelerator orchestrator. It is shipped with
high-level APIs in C/C++, Java, and Python, and a unified engine that supports
every heterogeneous multi-accelerator platform.

## Using the API

On the client side, the application instantiates local objects known as requests
that correspond to accelerators available at the server. The application can
then just populate those requests with data in familiar data types and
structures and submit them - Coral API looks after wrapping the arguments for
the accelerator in the appropriate format, sending the request(s) to the server
and returning the server’s response(s).

In many scenarios it’s useful to be able to start RPCs without blocking the
current thread. The Coral programming API in all languages comes in an
asynchronous flavor. You can find out more in each language’s tutorial.

## RPC life cycle

In this section, you’ll take a closer look at what happens when a Coral client
requests an accelerator. Let's consider the simplest type of RPC where the
client sends a single request and gets back a single response.

1. Once the client submits an accelerator request, Coral is notified that the
RPC has been invoked with the client’s metadata for this request, the
accelerator name, and the specified arguments.

2. Once the server has the client’s request, it does whatever work is necessary
to run the accelerator and populate a response. The response is then returned to
the client together with status details (status code) and optional trailing
messages.

3. Then the client gets the response and checks if the response status is OK,
which completes the call on the client side.

# Quick start

{{< lang c >}}
This guide gets you started with Coral API in C with a simple working example.
{{< /lang >}}

{{< lang cpp >}}
This guide gets you started with Coral API in C++ with a simple working example.
{{< /lang >}}

{{< lang java >}}
This guide gets you started with Coral API in Java with a simple working
example.
{{< /lang >}}

{{< lang python >}}
This guide gets you started with Coral API in Python with a simple working
example.
{{< /lang >}}

## Example program

```c
#include <inaccel/coral.h>
```

```cpp
#include <inaccel/coral>
```

```java
import com.inaccel.coral.*;

import io.netty.buffer.ByteBuf;

import java.util.concurrent.Future;
```

```python
import inaccel.coral as inaccel
import numpy as np
```

{{< lang c >}}
In this section, we’ll look at creating a C client for a vector addition
accelerator.

You can see our complete example client code in
[coral-api/src/test/c/example.c](https://github.com/inaccel/coral-api/blob/master/src/test/c/example.c).
{{< /lang >}}

{{< lang cpp >}}
In this section, we’ll look at creating a C++ client for a vector addition
accelerator.

You can see our complete example client code in
[coral-api/src/test/cpp/example.cpp](https://github.com/inaccel/coral-api/blob/master/src/test/cpp/example.cpp).
{{< /lang >}}

{{< lang java >}}
In this section, we’ll look at creating a Java client for a vector addition
accelerator.

You can see our complete example client code in
[coral-api/src/test/java/Example.java](https://github.com/inaccel/coral-api/blob/master/src/test/java/Example.java).
{{< /lang >}}

{{< lang python >}}
In this section, we’ll look at creating a Python client for a vector addition
accelerator.

You can see our complete example client code in
[coral-api/src/test/python/example.py](https://github.com/inaccel/coral-api/blob/master/src/test/python/example.py).
{{< /lang >}}

## Supported argument types

An argument can be a scalar data type (single value; such as an int, a float, or
a user-defined structure), or an array data type (one-dimensional collection of
values).

### Scalar arguments

```c
	int size = 1024;
```

```cpp
	int size = 1024;
```

```java
		int size = 1024;
```

```python
size = np.int32(1024)
```

For example, a 32-bit integer value.

### Array arguments

Shared memory objects stored as a block of contiguous memory and used to hold
data in a Coral program. Coral API maintains memory consistency in a
coarse-grained fashion in regions of arrays. Memory consistency is guaranteed at
synchronization points.

To allocate a zero-initialized array that can be shared by the host and all
accelerators, use the language-native InAccel allocator:

> allocate

```c
	float *a = (float *) inaccel_alloc(size * sizeof(float));
	float *b = (float *) inaccel_alloc(size * sizeof(float));
	float *c = (float *) inaccel_alloc(size * sizeof(float));
```

```cpp
	inaccel::vector<float> a(size), b(size), c(size);
```

```java
		ByteBuf a = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);
		ByteBuf b = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);
		ByteBuf c = InAccelByteBufAllocator.DEFAULT.buffer(size * Float.BYTES);
```

```python
with inaccel.allocator:
    a = np.ndarray(size, dtype=np.float32)
    b = np.ndarray(size, dtype=np.float32)
    c = np.ndarray(size, dtype=np.float32)
```

{{< lang c >}}
The array can be manipulated much as one would with any block of memory in C.
{{< /lang >}}

{{< lang cpp >}}
The array (`vector` object) can be manipulated much as one would with any block
of memory in C++.
{{< /lang >}}

{{< lang java >}}
The array (`ByteBuf` object) can be manipulated much as one would with any block
of memory in Java.
{{< /lang >}}

{{< lang python >}}
The array (`ndarray` object) can be manipulated much as one would with any block
of memory in Python.
{{< /lang >}}

## Defining the request

```c
	inaccel_request request = inaccel_request_create("vector.addition");
```

```cpp
	inaccel::request request("vector.addition");
```

```java
		InAccel.Request request = new InAccel.Request("vector.addition");
```

```python
request = inaccel.request('vector.addition')
```

Our next step is to instantiate the accelerator request and define the argument
list. To define a request, you specify the name of the target accelerator.

For this example, consider the following accelerator:

`void vadd(const float *a, const float *b, float *c, int size) { ... }`

To submit a request, the accelerator arguments must be set. To set the argument
value for a specific argument of an accelerator, call:

```c
	inaccel_request_arg_array(request, size * sizeof(float), a, 0);
	inaccel_request_arg_array(request, size * sizeof(float), b, 1);
	inaccel_request_arg_array(request, size * sizeof(float), c, 2);
	inaccel_request_arg_scalar(request, sizeof(int), &size, 3);
```

```cpp
	request.arg(a).arg(b).arg(c).arg(size);
```

```java
		request.arg(a).arg(b).arg(c).arg(size);
```

```python
request.arg(a).arg(b).arg(c).arg(size)
```

If the accelerator argument is declared to be a pointer of a built-in, or a
user-defined type, the specified argument value must be an appropriate array
region. For all other accelerator arguments (scalars), it must be a reference to
the actual data.

{{< aside notice >}}
Argument index values for `vadd` will be 0 for `a`, 1 for `b`, 2 for `c` and 3
for `size`.
{{< /aside >}}

## Submit the request

To enqueue a request to execute an accelerator, call the `submit` function:

{{< lang c >}}
```c
	inaccel_response response = inaccel_response_create();
```
{{< /lang >}}

```c
	inaccel_submit(request, response);
```

```cpp
	std::future<void> response = inaccel::submit(request);
```

```java
		Future<Void> response = InAccel.submit(request);
```

```python
response = inaccel.submit(request)
```

The response object is used to track the status of a request.

## Wait for the response

```c
	inaccel_response_wait(response);
```

```cpp
	response.get();
```

```java
		response.get();
```

```python
response.result()
```

This function waits on the host thread for requests identified by response
objects to complete. The responses act as synchronization points.

{{< lang c >}}
```c
	inaccel_response_release(response);

	inaccel_request_release(request);
```

> free

```c
	inaccel_free(c);
	inaccel_free(b);
	inaccel_free(a);
```
{{< /lang >}}

{{< lang java >}}
> free

```java
		c.release();
		b.release();
		a.release();
```
{{< /lang >}}

## Try it out!

{{< lang c >}}
> Compile the example:

```shell
gcc src/test/c/example.c -lcoral-api
```

> Run the example:

```shell
./a.out
```

| Requirements |
| ------------ |
| gcc          |
{{< /lang >}}

{{< lang cpp >}}
> Compile the example:

```shell
g++ src/test/cpp/example.cpp -lcoral-api -pthread
```

> Run the example:

```shell
./a.out
```

| Requirements |
| ------------ |
| c++ 11+, gcc |
{{< /lang >}}

{{< lang java >}}
> Compile and run the example:

```shell
gradle example
```

| Requirements    |
| --------------- |
| gradle, java 8+ |
{{< /lang >}}

{{< lang python >}}
> Run the example:

```shell
python src/test/python/example.py
```

| Requirements |
| ------------ |
| python 3.7+  |
{{< /lang >}}

Congratulations! You’ve just run an accelerated application with Coral.
