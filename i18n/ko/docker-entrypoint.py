#!/usr/bin/env python3
"""Book-independent Docker adapter for the public, pinned Korean toolchain."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

BOOK = Path('/guide')
OUTPUT = Path('/output')
COMMON = Path('/ko')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--format', choices=['all', 'html', 'pdf'], default='all')
    parser.add_argument('--pdf-targets', default='all')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--lang', help='Override the book content language')
    parser.add_argument('--status', action='store_true')
    parser.add_argument('--clean', action='store_true')
    args = parser.parse_args()
    config = json.loads((BOOK / 'i18n/ko/build.json').read_text())
    profile = config['profile']
    if profile not in ['bgnet', 'bggit']:
        raise ValueError('unsupported guide profile')
    # The image must correspond to this book's lock, even for read-only status.
    lock = json.loads((BOOK / 'i18n/ko/toolchain.lock').read_text())
    for key, repo in [('bgbspd_ko', COMMON), ('bgbspd', Path('/bgbspd'))]:
        revision = subprocess.check_output(['git', '-C', str(repo), 'rev-parse', 'HEAD'], text=True).strip()
        if revision != lock[key]:
            raise ValueError('toolchain.lock changed; rebuild the Docker image')
    if args.status:
        baseline = BOOK / 'i18n/ko/upstream-baseline.json'
        if not baseline.exists():
            print('This existing translation has no source baseline manifest; nothing was initialized or overwritten.')
            return 0
        return subprocess.call(['python3', str(COMMON / 'bin/translation.py'), 'status',
                                '--book', str(BOOK), '--guide-id', profile])
    if args.clean:
        if not OUTPUT.exists() or not any(OUTPUT.iterdir()):
            return 0
        marker = OUTPUT / '.bgbspd-ko-output'
        if not marker.is_file() or marker.read_text().strip() != profile:
            raise ValueError('refusing to clean output not owned by this guide')
        for child in OUTPUT.iterdir():
            if child.is_dir() and not child.is_symlink():
                shutil.rmtree(child)
            else:
                child.unlink()
        return 0
    marker = OUTPUT / '.bgbspd-ko-output'
    if marker.exists() and marker.read_text().strip() != profile:
        raise ValueError('output directory belongs to another guide')
    command = ['python3', str(COMMON / 'bin/build-guide.py'), '--book', str(BOOK),
               '--profile', profile, '--output', str(OUTPUT), '--bgbspd', '/bgbspd',
               '--format', args.format, '--pdf-targets', args.pdf_targets,
               '--jobs', str(args.jobs), '--lang', args.lang or config['lang']]
    if config.get('extra_head'):
        head = (BOOK / config['extra_head']).resolve()
        if BOOK not in head.parents:
            raise ValueError('extra_head must be inside the book')
        command.extend(['--extra-head', str(head)])
    result = subprocess.call(command)
    if result:
        return result
    # Preserve bgnet's published entry points; apply the same layout to bggit.
    html = OUTPUT / 'html'
    if args.format in ['all', 'html']:
        for source, alias in [(f'{profile}.html', 'index.html'),
                              (f'{profile}-wide.html', 'index-wide.html'),
                              (f'{profile}-split.zip', f'{profile}.zip'),
                              (f'{profile}-split-wide.zip', f'{profile}-wide.zip')]:
            shutil.copy2(html / source, html / alias)
    source = OUTPUT / 'source'
    if source.is_dir():
        archive = source / f'{profile}_source.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
            for file in sorted(source.rglob('*')):
                if file.is_file() and file != archive:
                    z.write(file, Path(f'{profile}_source') / file.relative_to(source))
        if (html / 'source').is_dir():
            shutil.copy2(archive, html / 'source' / archive.name)
    if args.format in ['all', 'html']:
        return subprocess.call(['python3', str(COMMON / 'bin/check-html.py'), str(html)])
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as exc:
        sys.exit(str(exc))
