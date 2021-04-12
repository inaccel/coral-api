import ctypes as _ctypes
import os as _os

_path = 'native/lib{}.so'


def load(libname, file):
    name = _path.format(libname)

    return _ctypes.CDLL(_os.path.join(_os.path.dirname(file), name), use_errno=True)
