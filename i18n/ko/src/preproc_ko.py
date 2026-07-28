#!/usr/bin/env python3

import importlib.util
import os
import runpy
import sys


def load_preproc_config(build_dir):
    config_path = os.path.join(build_dir, "bin", "preproc_config.py")
    spec = importlib.util.spec_from_file_location("preproc_config", config_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    build_dir = os.environ.get("BGBSPD_BUILD_DIR", "../../../../bgbspd")
    config = load_preproc_config(build_dir)

    outfile = sys.argv[-1] if len(sys.argv) > 1 else ""

    if "_html" in os.path.basename(outfile):
        config.EXAMPLE_URL = "../source/examples/"

    sys.modules["preproc_config"] = config
    runpy.run_path(os.path.join(build_dir, "bin", "preproc"), run_name="__main__")


if __name__ == "__main__":
    main()
