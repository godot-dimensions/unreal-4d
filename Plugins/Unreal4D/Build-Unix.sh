#!/bin/sh

# Packages the Unreal4D plugin on macOS or Linux.

set -eu

engine_root=
package_dir=
target_platforms=

usage() {
    printf 'Usage: %s [--engine-root PATH] [--package-dir PATH] [--target-platforms PLATFORMS]\n' "$0"
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
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "Unknown argument: $1"
            ;;
    esac
done

case "$(uname -s)" in
    Darwin)
        host_platform=Mac
        ;;
    Linux)
        host_platform=Linux
        ;;
    *)
        die "This script supports macOS and Linux only."
        ;;
esac

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)

if [ -z "$engine_root" ]; then
    engine_root=${UNREAL_ENGINE_ROOT:-}
fi
if [ -z "$engine_root" ]; then
    engine_root=$script_dir/../../../UnrealEngine6
fi
[ -d "$engine_root" ] ||
    die "Unreal Engine was not found at '$engine_root'. Pass --engine-root or set UNREAL_ENGINE_ROOT."
engine_root=$(CDPATH= cd -P "$engine_root" && pwd)

. "$script_dir/Tools/MacEnvironment.sh"
unreal4d_setup_environment "$script_dir" "$engine_root"

plugin_file=$script_dir/Unreal4D.uplugin
run_uat=$engine_root/Engine/Build/BatchFiles/RunUAT.sh

[ -f "$plugin_file" ] || die "Plugin descriptor not found: $plugin_file"
[ -f "$run_uat" ] || die "RunUAT.sh was not found beneath '$engine_root'."

if [ -z "$target_platforms" ]; then
    target_platforms=$host_platform
fi
if [ -z "$package_dir" ]; then
    package_dir=$script_dir/BuildArtifacts/Unreal4D-$target_platforms
else
    case "$package_dir" in
        /*) ;;
        *) package_dir=$(pwd)/$package_dir ;;
    esac
fi

unreal4d_run "$run_uat" BuildPlugin \
    "-Plugin=$plugin_file" \
    "-Package=$package_dir" \
    "-TargetPlatforms=$target_platforms"

printf "Successfully packaged Unreal4D to '%s'.\n" "$package_dir"
