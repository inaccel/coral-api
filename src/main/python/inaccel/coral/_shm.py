from . import _library

import ctypes as _ctypes
import numpy_allocator as _numpy_allocator

_c = _library.load('coral-api', __file__)

_c.PyDataMemType_CallocFunc.argtypes = [_ctypes.c_size_t, _ctypes.c_size_t]
_c.PyDataMemType_CallocFunc.restype = _ctypes.c_void_p

_c.PyDataMemType_FreeFunc.argtypes = [_ctypes.c_void_p, _ctypes.c_size_t]
_c.PyDataMemType_FreeFunc.restype = None

_c.PyDataMemType_MallocFunc.argtypes = [_ctypes.c_size_t]
_c.PyDataMemType_MallocFunc.restype = _ctypes.c_void_p

_c.PyDataMemType_ReallocFunc.argtypes = [_ctypes.c_void_p, _ctypes.c_size_t]
_c.PyDataMemType_ReallocFunc.restype = _ctypes.c_void_p


class inaccel_allocator(metaclass=_numpy_allocator.type):

    _calloc_ = _c.PyDataMemType_CallocFunc

    _free_ = _c.PyDataMemType_FreeFunc

    _malloc_ = _c.PyDataMemType_MallocFunc

    _realloc_ = _c.PyDataMemType_ReallocFunc


allocator = inaccel_allocator
