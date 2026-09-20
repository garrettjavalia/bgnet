#!/bin/sh
# Fetch one immutable tool commit; all build logic belongs to that checkout.
set -eu
revision=${1:?missing commit}
destination=${2:?missing cache directory}
repository=https://github.com/garrettjavalia/bgbspd-ko.git
case "$revision" in ''|*[!0-9a-f]*) echo 'Expected a full lowercase commit SHA' >&2; exit 1;; esac
test "${#revision}" -eq 40 || { echo 'Expected a 40-character commit SHA' >&2; exit 1; }
if test -e "$destination"; then
    test "$(git -C "$destination" rev-parse HEAD)" = "$revision"
    test "$(git -C "$destination" remote get-url origin)" = "$repository"
    test -z "$(git -C "$destination" status --porcelain)" || {
        echo 'Cached toolchain has local edits; use a clean checkout.' >&2; exit 1;
    }
    exit 0
fi
parent=$(dirname "$destination")
mkdir -p "$parent"
temporary=$(mktemp -d "$parent/.fetch.XXXXXX")
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
git init -q "$temporary"
git -C "$temporary" remote add origin "$repository"
git -C "$temporary" fetch --depth 1 origin "$revision"
git -C "$temporary" checkout -q --detach FETCH_HEAD
test "$(git -C "$temporary" rev-parse HEAD)" = "$revision"
mv "$temporary" "$destination"
trap - EXIT HUP INT TERM
