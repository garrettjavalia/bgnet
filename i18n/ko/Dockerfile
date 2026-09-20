FROM python:3.12-slim

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y git make pandoc texlive-xetex zip && apt-get clean

COPY i18n/ko/toolchain.lock /tmp/toolchain.lock
RUN python3 <<'PY'
import json
from pathlib import Path
import re
import subprocess

lock = json.loads(Path('/tmp/toolchain.lock').read_text())
for key, url, path in [
    ('bgbspd_ko', 'https://github.com/garrettjavalia/bgbspd-ko.git', '/ko'),
    ('bgbspd', 'https://github.com/beejjorgensen/bgbspd.git', '/bgbspd'),
]:
    revision = lock[key]
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise SystemExit(f'{key}: expected a full commit SHA')
    subprocess.run(['git', 'init', path], check=True)
    subprocess.run(['git', '-C', path, 'remote', 'add', 'origin', url], check=True)
    subprocess.run(['git', '-C', path, 'fetch', '--depth', '1', 'origin', revision], check=True)
    subprocess.run(['git', '-C', path, 'checkout', '--detach', 'FETCH_HEAD'], check=True)
if Path('/ko/upstream.lock').read_text().strip() != lock['bgbspd']:
    raise SystemExit('book lock and shared tool disagree on bgbspd revision')
PY

COPY i18n/ko/docker-entrypoint.py /usr/local/bin/build-korean-guide
WORKDIR /guide
ENTRYPOINT ["python3", "/usr/local/bin/build-korean-guide"]
