#!/bin/sh

# Set up the macOS warning and Metal environment, then execute COMMAND.
# Generated IDE tasks pass the engine root and the build command as arguments.

set -eu

[ "$#" -ge 2 ] || { printf 'Usage: %s ENGINE_ROOT COMMAND [ARGS...]\n' "$0" >&2; exit 1; }
script_dir=$(CDPATH= cd -P "$(dirname "$0")" && pwd)
engine_root=$1
shift
. "$script_dir/MacEnvironment.sh"
unreal4d_setup_environment "$script_dir/.." "$engine_root"

case "$1" in
    */Mac/Build.sh)
        case "${2:-}" in
            *Editor)
                unreal4d_build_worker=1
                for unreal4d_argument in "$@"; do
                    case "$unreal4d_argument" in
                        -[Cc]lean|-buildscw|-SkipBuild) unreal4d_build_worker=0 ;;
                    esac
                done
                if [ "$unreal4d_build_worker" -eq 1 ]; then
                    set -- "$@" -buildscw
                fi
                ;;
        esac
        ;;
esac
unreal4d_exec "$@"
