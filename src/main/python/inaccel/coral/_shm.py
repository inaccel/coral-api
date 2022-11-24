from . import _library

import ctypes as _ctypes
import numpy_allocator as _numpy_allocator

_c = _library.load('coral-api', __file__)


class inaccel_allocator(metaclass=_numpy_allocator.type):
    _calloc_ = _ctypes.addressof(_c.PyDataMem_CallocFunc)
    _free_ = _ctypes.addressof(_c.PyDataMem_FreeFunc)
    _malloc_ = _ctypes.addressof(_c.PyDataMem_MallocFunc)
    _realloc_ = _ctypes.addressof(_c.PyDataMem_ReallocFunc)


allocator = inaccel_allocator
