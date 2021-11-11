from . import _library

import numpy_allocator as _numpy_allocator

_c = _library.load('coral-api', __file__)


class inaccel_allocator(metaclass=_numpy_allocator.type):

    _calloc_ = _c.PyDataMemType_CallocFunc

    _free_ = _c.PyDataMemType_FreeFunc

    _malloc_ = _c.PyDataMemType_MallocFunc

    _realloc_ = _c.PyDataMemType_ReallocFunc


allocator = inaccel_allocator
