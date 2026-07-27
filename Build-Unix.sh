#!/bin/sh

# Packages the Unreal4D plugin, then generates metadata and builds the example.

set -eu

engine_root=
package_dir=
target_platforms=
target=Editor
configuration=Development
skip_project_files=0
generate_only=0

usage() {
    printf 'Usage: %s [--engine-root PATH] [--package-dir PATH] [--target-platforms PLATFORMS] [--target Editor|Game] [--configuration Development|DebugGame|Shipping] [--skip-project-files] [--generate-only]\n' "$0"
}

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 1
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --engine-root)
            [ "$#" -ge 2 ] || die "--engine-root requires a value."
            engine_root=$2
            shift 2
            ;;
        --package-dir)
            [ "$#" -ge 2 ] || die "--package-dir requires a value."
            package_dir=$2
            shift 2
            ;;
        --target-platforms)
            [ "$#" -ge 2 ] || die "--target-platforms requires a value."
            target_platforms=$2
            shift 2
            ;;
        --target)
            [ "$#" -ge 2 ] || die "--target requires a value."
            target=$2
            shift 2
            ;;
        --configuration)
            [ "$#" -ge 2 ] || die "--configuration requires a value."
            configuration=$2
            shift 2
            ;;
        --skip-project-files)
            skip_project_files=1
            shift
            ;;
        --generate-only)
            generate_only=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Unknown argument: $1"
            ;;
    esac
done

if [ "$skip_project_files" -eq 1 ] && [ "$generate_only" -eq 1 ]; then
    die "--skip-project-files and --generate-only cannot be used together."
fi

case "$(uname -s)" in
    Darwin|Linux) ;;
    *) die "This script supports macOS and Linux only." ;;
esac

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
plugin_script=$script_dir/Plugins/Unreal4D/Build-Unix.sh
example_script=$script_dir/Examples/Unreal4DExample/Build-Unix.sh

[ -f "$plugin_script" ] || die "Plugin build script not found: $plugin_script"
[ -f "$example_script" ] || die "Example build script not found: $example_script"

set -- "$plugin_script"
[ -z "$engine_root" ] || set -- "$@" --engine-root "$engine_root"
[ -z "$package_dir" ] || set -- "$@" --package-dir "$package_dir"
[ -z "$target_platforms" ] || set -- "$@" --target-platforms "$target_platforms"

printf '===============================================\n'
printf 'Unreal4D Unix Build\n'
printf '===============================================\n'
printf '\nStep 1 of 2: Packaging the Unreal4D plugin...\n'
"$@"

set -- "$example_script" --target "$target" --configuration "$configuration"
[ -z "$engine_root" ] || set -- "$@" --engine-root "$engine_root"
[ "$skip_project_files" -eq 0 ] || set -- "$@" --skip-project-files
[ "$generate_only" -eq 0 ] || set -- "$@" --generate-only

printf '\nStep 2 of 2: Setting up VS Code and building the example...\n'
"$@"

printf '\n===============================================\n'
printf 'Unreal4D plugin and example are ready.\n'
printf '===============================================\n'
