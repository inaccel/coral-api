from numpy_allocator import base_allocator as _base_allocator
from . import _library

_c = _library.load('coral-api', __file__)


class allocator(metaclass=_base_allocator):

    _alloc_ = _c.PyDataMem_AllocFunc

    _free_ = _c.PyDataMem_FreeFunc

    _realloc_ = _c.PyDataMem_ReallocFunc

    _zeroed_alloc_ = _c.PyDataMem_ZeroedAllocFunc
