#!/usr/bin/env python3
# -*- coding: utf-8 -*-

assert "__main__" != __name__


def _private():
    from .. import common as _common_module

    _make_lazy = _common_module.lazy_attributes.make_getter
    _make_property_collector = _common_module.property_collector.make

    return _make_property_collector(lazy = _make_lazy(dictionary = dict(
        make = lambda module: getattr(module, "_class").make,
        Class = lambda module: getattr(module, "_class").Class
    )))


_private = _private()

__all__ = _private.lazy.keys
__date__ = None
__author__ = None
__version__ = None
__credits__ = None
_fields = tuple()
__bases__ = tuple()


def __getattr__(name: str): return _private.lazy(name = name)
