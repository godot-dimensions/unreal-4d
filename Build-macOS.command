#!/bin/sh

# Double-click this file to package the plugin and build the example on macOS.

set -u

die_and_wait() {
    printf '\nERROR: %s\n' "$*" >&2
    printf 'Press Return to close this window...'
    read ignored
    exit 1
}

[ "$(uname -s)" = Darwin ] || die_and_wait "This launcher requires macOS."

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
build_script=$script_dir/Build-Unix.sh

[ -f "$build_script" ] || die_and_wait "Unix build script not found: $build_script"

"$build_script" "$@"
exit_code=$?

if [ "$exit_code" -ne 0 ]; then
    printf '\nThe Unreal4D build failed. Review the error above.\n' >&2
    printf 'Press Return to close this window...'
    read ignored
fi

exit "$exit_code"
