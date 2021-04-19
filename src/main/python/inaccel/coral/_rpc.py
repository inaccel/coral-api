from . import _library
from ._shm import allocator as _allocator

import concurrent.futures as _futures
import ctypes as _ctypes
import numpy as _numpy
import os as _os

_c = _library.load('coral-api', __file__)

_c.inaccel_request_arg_array.argtypes = [_ctypes.c_void_p, _ctypes.c_size_t, _ctypes.c_void_p, _ctypes.c_uint]
_c.inaccel_request_arg_array.restype = _ctypes.c_int

_c.inaccel_request_arg_scalar.argtypes = [_ctypes.c_void_p, _ctypes.c_size_t, _ctypes.c_void_p, _ctypes.c_uint]
_c.inaccel_request_arg_scalar.restype = _ctypes.c_int

_c.inaccel_request_create.argtypes = [_ctypes.c_char_p]
_c.inaccel_request_create.restype = _ctypes.c_void_p

_c.inaccel_request_snprint.argtypes = [_ctypes.c_char_p, _ctypes.c_size_t, _ctypes.c_void_p]
_c.inaccel_request_snprint.restype = _ctypes.c_int

_c.inaccel_request_release.argtypes = [_ctypes.c_void_p]

_c.inaccel_response_create.restype = _ctypes.c_void_p

_c.inaccel_response_release.argtypes = [_ctypes.c_void_p]

_c.inaccel_response_snprint.argtypes = [_ctypes.c_char_p, _ctypes.c_size_t, _ctypes.c_void_p]
_c.inaccel_response_snprint.restype = _ctypes.c_int

_c.inaccel_response_wait.argtypes = [_ctypes.c_void_p]
_c.inaccel_response_wait.restype = _ctypes.c_int

_c.inaccel_submit.argtypes = [_ctypes.c_void_p, _ctypes.c_void_p]
_c.inaccel_submit.restype = _ctypes.c_int

_executor = _futures.ThreadPoolExecutor()


class request:

    def __del__(self):
        if hasattr(self, '_c'):
            _c.inaccel_request_release(self._c)

    def __init__(self, accelerator):
        self._index = 0

        self._c = _c.inaccel_request_create(accelerator.encode('utf-8'))
        if self._c is None:
            raise RuntimeError(_os.strerror(_ctypes.get_errno()))

    def __str__(self):
        n = _c.inaccel_request_snprint(None, 0, self._c)
        if n < 0:
            raise RuntimeError(_os.strerror(_ctypes.get_errno()))

        s = bytearray(n + 1)
        if _c.inaccel_request_snprint((_ctypes.c_char * len(s)).from_buffer(s), len(s), self._c) != n:
            raise RuntimeError(_os.strerror(_ctypes.get_errno()))

        return s.decode()

    def arg(self, value, index=None):
        if not index:
            index = self._index
            self._index += 1

        if isinstance(value, _numpy.ndarray):
            if _allocator.handles(value):
                error = _c.inaccel_request_arg_array(self._c, value.nbytes, value.__array_interface__['data'][0], index)
            else:
                error = _c.inaccel_request_arg_scalar(self._c, value.nbytes, value.__array_interface__['data'][0], index)
            if error:
                raise RuntimeError(_os.strerror(_ctypes.get_errno()))
        elif isinstance(value, _numpy.bool):
            error = _c.inaccel_request_arg_scalar(self._c, 1, value.to_bytes(1, 'little'), index)
            if error:
                raise RuntimeError(_os.strerror(_ctypes.get_errno()))
        elif isinstance(value, (_numpy.integer, _numpy.floating, _numpy.complexfloating)):
            error = _c.inaccel_request_arg_scalar(self._c, value.nbytes, value.newbyteorder('L').tobytes(), index)
            if error:
                raise RuntimeError(_os.strerror(_ctypes.get_errno()))
        else:
            raise ValueError()

        return self


def submit(request):
    cresponse = _c.inaccel_response_create()
    if cresponse is None:
        raise RuntimeError(_os.strerror(_ctypes.get_errno()))

    error = _c.inaccel_submit(request._c, cresponse)
    if error:
        errsv = _ctypes.get_errno()

        _c.inaccel_response_release(cresponse)

        raise RuntimeError(_os.strerror(errsv))

    def fn(cresponse):
        error = _c.inaccel_response_wait(cresponse)
        if error == -1:
            errsv = _ctypes.get_errno()

            _c.inaccel_response_release(cresponse)

            raise RuntimeError(_os.strerror(errsv))
        elif error:
            n = _c.inaccel_response_snprint(None, 0, cresponse)
            if n < 0:
                errsv = _ctypes.get_errno()

                _c.inaccel_response_release(cresponse)

                raise RuntimeError(_os.strerror(errsv))

            s = bytearray(n + 1)
            if _c.inaccel_response_snprint((_ctypes.c_char * len(s)).from_buffer(s), len(s), cresponse) != n:
                errsv = _ctypes.get_errno()

                _c.inaccel_response_release(cresponse)

                raise RuntimeError(_os.strerror(errsv))

            _c.inaccel_response_release(cresponse)

            raise Exception(s.decode())

        _c.inaccel_response_release(cresponse)

    return _executor.submit(fn, cresponse)
