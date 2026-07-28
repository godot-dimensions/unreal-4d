#!/bin/sh

# Builds the Unreal4D example, then opens it in the Unreal Editor.

set -eu

skip_build=0

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

engine_root=
expect_engine_root=0
for argument in "$@"; do
    if [ "$expect_engine_root" -eq 1 ]; then
        case "$argument" in
            --*) die "--engine-root requires a path." ;;
            *) engine_root=$argument ;;
        esac
        expect_engine_root=0
        continue
    fi

    case "$argument" in
        --skip-build)
            skip_build=1
            ;;
        --engine-root)
            expect_engine_root=1
            ;;
    esac
done

if [ "$expect_engine_root" -eq 1 ]; then
    die "--engine-root requires a path."
fi

if [ -z "$engine_root" ]; then
    engine_root=${UNREAL_ENGINE_ROOT:-}
fi
if [ -z "$engine_root" ]; then
    engine_root=$script_dir/../../../UnrealEngine5
fi

if [ "$skip_build" -eq 0 ]; then
    "$build_script" "$@"
fi

[ -d "$engine_root" ] ||
    die "Unreal Engine was not found at '$engine_root'. Pass --engine-root or set UNREAL_ENGINE_ROOT."
engine_root=$(CDPATH= cd -P "$engine_root" && pwd)

editor=$engine_root/$editor_relative
[ -x "$editor" ] || die "Unreal Editor was not found or is not executable: $editor"

printf 'Opening Unreal4DExample in the Unreal Editor...\n'
exec "$editor" "$project_file"
