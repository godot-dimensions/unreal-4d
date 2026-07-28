#!/bin/sh

# Double-click this file to build the Unreal4D example and open it on macOS.

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
run_script=$script_dir/Examples/Unreal4DExample/Run-macOS.command

[ -f "$run_script" ] || die_and_wait "Example run script not found: $run_script"

"$run_script" "$@"
exit_code=$?

if [ "$exit_code" -ne 0 ]; then
    printf '\nThe Unreal4D example could not be built or opened. Review the error above.\n' >&2
    printf 'Press Return to close this window...'
    read ignored
fi

exit "$exit_code"
