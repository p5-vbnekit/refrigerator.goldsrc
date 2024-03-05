#!/usr/bin/env python3

assert "__main__" == __name__

_this_file_path = __file__
assert isinstance(_this_file_path, str) and _this_file_path


def _private():
    import sys
    import json
    import pathlib
    import importlib
    import subprocess

    _base_path = pathlib.Path(_this_file_path).resolve(strict = True)
    assert _base_path.is_file()
    assert 2 < len(_base_path.relative_to(_base_path.anchor).parts)
    _base_path = _base_path.parent.parent

    def _auxiliary_module():
        _path = (_base_path / "lib/python3/_auxiliary").resolve(strict = True)
        assert _path.is_dir()
        _base = _path.parent.as_posix()
        _path = (_path / "__init__.py").resolve(strict = True)
        assert _path.is_file()
        assert f"{_base}/_auxiliary/__init__.py" == _path.as_posix()
        sys.path.append(_base)
        try: _module = importlib.import_module("_auxiliary")
        finally: sys.path.remove(_base)
        assert _path == pathlib.Path(_module.__file__)
        return _module

    _auxiliary_module = _auxiliary_module()

    def _load_json(path: pathlib.Path):
        assert path.resolve(strict = True) == path
        assert path.is_file()
        with open(path, mode = "r") as _stream:
            _data = _stream.read()
            assert not _stream.read()
        _data = json.loads(_data)
        assert isinstance(_data, dict)
        return json.dumps(_data)

    def _run():
        _config = _auxiliary_module.common.property_collector.make(
            directory = pathlib.Path.cwd().resolve(strict = True)
        )

        _config.action = sys.argv[1:]
        if _config.action:
            _config.action, = _config.action
            assert _config.action in {"setconfig", "guiconfig", "menuconfig"}
        else: _config.action = "setconfig"

        _config.native = _config.directory / ".config"
        if _config.native.exists():
            assert _config.native.resolve(strict = True) == _config.native
            assert _config.native.is_file()

        _config.json = _auxiliary_module.common.property_collector.make(
            path = _config.directory / f"{_config.native.name}.json"
        )
        if _config.json.path.exists(): _config.json.previous = _load_json(path = _config.json.path)
        else: _config.json.previous = json.dumps(dict())

        _config.scheme = _config.directory / "KConfig"
        assert _config.scheme.resolve(strict = True) == _config.scheme
        assert _config.scheme.is_file()
        _config.scheme = _config.scheme.as_posix()

        _config.utility = _auxiliary_module.kconfig.make(source = _config.scheme)
        _config.action = sys.executable, "--", *getattr(_config.utility, _config.action)
        _config.utility = _config.utility.library.Kconfig(_config.scheme)
        subprocess.check_call(_config.action, cwd = _config.directory.as_posix())
        assert _config.native.resolve(strict = True) == _config.native
        assert _config.native.is_file()
        _config.utility.load_config(_config.native.as_posix())
        _config.utility = _config.utility.defined_syms

        _config.json.current = json.dumps({_c.name: _c.str_value for _c in _config.utility})
        if _config.json.current == _config.json.previous: return
        with open(_config.json.path, mode = "w") as _stream: print(_config.json.current, file = _stream)

    return _auxiliary_module.common.property_collector.make(run = _run)


try: _private().run()
finally: del _private
