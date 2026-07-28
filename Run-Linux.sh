#!/bin/sh

# Builds the Unreal4D example and opens it in the Unreal Editor on Linux.

set -eu

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

[ "$(uname -s)" = Linux ] || die "This launcher requires Linux."

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
run_script=$script_dir/Examples/Unreal4DExample/Run-Linux.sh

[ -f "$run_script" ] || die "Example run script not found: $run_script"

exec "$run_script" "$@"
