#!/bin/sh

# Packages Unreal4D on Linux.

set -eu

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

[ "$(uname -s)" = Linux ] || die "This launcher requires Linux."

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
build_script=$script_dir/Build-Unix.sh

[ -f "$build_script" ] || die "Unix build script not found: $build_script"

exec "$build_script" "$@"
