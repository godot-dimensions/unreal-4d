#!/bin/sh

# Builds the Unreal4D example, then opens it in the Unreal Editor.

set -eu

skip_build=0
generate_only=0
configuration=Development
engine_root=

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

case "$(uname -s)" in
    Darwin)
        editor_relative=Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor
        ;;
    Linux)
        editor_relative=Engine/Binaries/Linux/UnrealEditor
        ;;
    *)
        die "This script supports macOS and Linux only."
        ;;
esac

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
build_script=$script_dir/Build-Unix.sh
project_file=$script_dir/Unreal4DExample.uproject

[ -f "$build_script" ] || die "Unix build script not found: $build_script"
[ -f "$project_file" ] || die "Project descriptor not found: $project_file"

parse_arguments() {
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --engine-root|--target|--configuration)
                [ "$#" -ge 2 ] || die "$1 requires a value."
                case "$1" in
                    --engine-root) engine_root=$2 ;;
                    --target)
                        case "$2" in
                            Editor|editor|Game|game) ;;
                            *) die "--target must be Editor or Game." ;;
                        esac
                        ;;
                    --configuration)
                        case "$2" in
                            Development|DebugGame|Shipping) configuration=$2 ;;
                            *) die "--configuration must be Development, DebugGame, or Shipping." ;;
                        esac
                        ;;
                esac
                shift 2
                ;;
            --skip-build) skip_build=1; shift ;;
            --skip-project-files) shift ;;
            --generate-only) generate_only=1; shift ;;
            -h|--help)
                printf 'Usage: %s [--engine-root PATH] [--target Editor|Game] [--configuration Development|DebugGame|Shipping] [--skip-project-files] [--generate-only] [--skip-build]\n' "$0"
                exit 0
                ;;
            *) die "Unknown argument: $1" ;;
        esac
    done
}
parse_arguments "$@"

if [ "$skip_build" -eq 1 ] && [ "$generate_only" -eq 1 ]; then
    die "--skip-build and --generate-only cannot be used together."
fi

if [ -z "$engine_root" ]; then
    engine_root=${UNREAL_ENGINE_ROOT:-}
fi
if [ -z "$engine_root" ]; then
    engine_root=$script_dir/../../../UnrealEngine5
fi

[ -d "$engine_root" ] ||
    die "Unreal Engine was not found at '$engine_root'. Pass --engine-root or set UNREAL_ENGINE_ROOT."
engine_root=$(CDPATH= cd -P "$engine_root" && pwd)
plugin_root=$script_dir/../../Plugins/Unreal4D
. "$plugin_root/Tools/MacEnvironment.sh"
unreal4d_setup_environment "$plugin_root" "$engine_root"

if [ "$skip_build" -eq 0 ]; then
    unreal4d_run "$build_script" "$@"
fi

if [ "$generate_only" -eq 1 ]; then
    exit 0
fi

editor=$engine_root/$editor_relative
[ -x "$editor" ] || die "Unreal Editor was not found or is not executable: $editor"

printf 'Opening Unreal4DExample in the Unreal Editor...\n'
if [ "$configuration" = DebugGame ]; then
    unreal4d_exec "$editor" "$project_file" -debug
fi
unreal4d_exec "$editor" "$project_file"
