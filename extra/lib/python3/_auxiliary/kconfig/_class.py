#!/usr/bin/env python3
# -*- coding: utf-8 -*-

assert "__main__" != __name__


def _private():
    _location = __file__

    import pathlib

    from .. import common as _common_module

    _make_property_collector = _common_module.property_collector.make

    _location = pathlib.Path(_location).resolve(strict = True)
    assert _location.is_file()
    _location = (_location.parent / "_lib").resolve(strict = True)
    assert _location.is_dir()

    def _library():
        from . import _lib as _namespace
        _namespace, = _namespace.__path__
        assert _location == pathlib.Path(_namespace)
        from . _lib import kconfiglib as _instance
        _instance_path = pathlib.Path(_instance.__file__).resolve(strict = True)
        assert _instance_path.is_file()
        assert (_location / "kconfiglib.py").as_posix() == _instance_path.as_posix()
        return _instance

    _library = _library()

    def _make_item(key: str, source: str):
        assert isinstance(key, str)
        assert isinstance(source, str)

        if str is not type(key): key = str(key)
        if str is not type(source): source = str(source)

        assert key in {"setconfig", "guiconfig", "menuconfig"}
        assert source

        if "setconfig" == key: source = f"--kconfig={source}"

        key = _location / f"{key}.py"
        assert key.resolve(strict = True).as_posix() == key.as_posix()
        assert key.is_file()
        key = key.as_posix()

        return key, source

    class _Class(object):
        library = _library

        def __getattr__(self, name: str): return _make_item(key = name, source = self.__source)

        def __init__(self, source: str):
            super().__init__()
            self.__source = source

    return _make_property_collector(Class = _Class, library = _library)


try: Class = _private().Class
finally: del _private

library = Class.library


def make(*args, **kwargs): return Class(*args, **kwargs)
