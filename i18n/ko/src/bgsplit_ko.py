#!/usr/bin/env python3
"""Run the shared splitter with a Korean navigation renderer."""

import importlib.util
import os
import sys


def load_splitter(build_dir):
    splitter_path = os.path.join(build_dir, "bin", "bgsplit.py")
    spec = importlib.util.spec_from_file_location("bgsplit", splitter_path)

    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load splitter from {splitter_path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def localize_navigation(splitter):
    def get_nav_ko(self):
        info = self.section_id_to_info[self.current_section_id]
        html = '<div style="text-align:center">'

        if info.prev_id is not None:
            file = splitter.file_name_for_section(info.prev_id)
            span_style = ""
        else:
            file = ""
            span_style = ' style="visibility: hidden"'

        html += f'<span{span_style}><a href="{file}" rel="prev">이전</a> | </span>'
        html += '<a href="index.html">목차</a>'

        if info.next_id is not None:
            file = splitter.file_name_for_section(info.next_id)
            span_style = ""
        else:
            file = ""
            span_style = ' style="visibility: hidden"'

        html += f'<span{span_style}> | <a href="{file}" rel="next">다음</a></span>'
        html += "</div>"

        return html

    splitter.SplitHTMLParser.get_nav = get_nav_ko


def main():
    build_dir = os.environ.get("BGBSPD_BUILD_DIR", "../../../../bgbspd")
    splitter = load_splitter(build_dir)
    localize_navigation(splitter)
    return splitter.main(sys.argv)


if __name__ == "__main__":
    sys.exit(main())
