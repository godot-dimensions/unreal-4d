#!/bin/sh

# Shared macOS setup for project generation, compilation, packaging, and launch.

unreal4d_setup_environment() {
    unreal4d_mac_enabled=0
    [ "$(uname -s)" = Darwin ] || return 0

    unreal4d_plugin_root=$1
    unreal4d_engine_root=$2
    unreal4d_environment_helper=$unreal4d_plugin_root/Tools/MacEnvironment.py
    if command -v python3 >/dev/null 2>&1; then
        unreal4d_environment_python=$(command -v python3)
    else
        unreal4d_environment_python=$unreal4d_engine_root/Engine/Binaries/ThirdParty/Python3/Mac/bin/python3
    fi
    if [ ! -x "$unreal4d_environment_python" ]; then
        printf 'ERROR: Python 3 is required to discover the macOS Metal toolchain.\n' >&2
        return 1
    fi

    UBT_EXTRA_ARGS=$("$unreal4d_environment_python" "$unreal4d_environment_helper" ubt-args) || return 1
    export UBT_EXTRA_ARGS
    unreal4d_metal_root=$("$unreal4d_environment_python" "$unreal4d_environment_helper" metal-root --plugin-root "$unreal4d_plugin_root") || return 1
    unreal4d_editor_args=$("$unreal4d_environment_python" "$unreal4d_environment_helper" editor-args --metal-root "$unreal4d_metal_root") || return 1
    unreal4d_mac_enabled=1
}

unreal4d_run() {
    if [ "$unreal4d_mac_enabled" -eq 1 ]; then
        env "UE-CmdLineArgs=$unreal4d_editor_args" UNREAL4D_MAC_ENVIRONMENT=1 "$@"
    else
        "$@"
    fi
}

unreal4d_exec() {
    if [ "$unreal4d_mac_enabled" -eq 1 ]; then
        exec env "UE-CmdLineArgs=$unreal4d_editor_args" UNREAL4D_MAC_ENVIRONMENT=1 "$@"
    else
        exec "$@"
    fi
}
