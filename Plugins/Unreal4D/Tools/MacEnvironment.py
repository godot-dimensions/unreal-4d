#!/usr/bin/env python3

"""Configure UE's supported environment hooks without changing the engine."""

import argparse
import glob
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


WARNING_FLAGS = (
    "-Wno-error=implicit-int-float-conversion",
    "-Wno-error=character-conversion",
)
METAL_TOOLS = ("metal", "metal-ar", "metallib", "metal-pack", "metal-strip")


def argument_tokens(value):
    # UBT_EXTRA_ARGS uses UE's double-quote parser, not shell single quotes or
    # shell expansion. Preserve unrelated arguments exactly as supplied.
    if value.count('"') % 2:
        raise ValueError("Unmatched double quote in Unreal environment arguments.")
    return re.findall(r'(?:[^\s"]|"[^"]*")+', value)


def ubt_args(value):
    arguments = []
    compiler_arguments = []
    for token in argument_tokens(value):
        decoded = token.replace('"', "")
        if decoded.lower().startswith("-compilerarguments="):
            compiler_arguments.append(decoded.split("=", 1)[1])
        else:
            arguments.append(token)
    compiler_arguments = " ".join(compiler_arguments).strip()
    for flag in WARNING_FLAGS:
        if flag not in compiler_arguments.split():
            compiler_arguments = (compiler_arguments + " " + flag).strip()
    arguments.append('-CompilerArguments="' + compiler_arguments + '"')
    return " ".join(arguments)


def editor_args(value, metal_root):
    arguments = [
        token for token in argument_tokens(value)
        if not token.replace('"', "").lower().startswith("-metaltoolchainoverride=")
    ]
    if '"' in str(metal_root):
        raise ValueError("The Metal toolchain path cannot contain a double quote.")
    arguments.append('-MetalToolchainOverride="' + str(metal_root) + '"')
    return " ".join(arguments)


def command_output(arguments):
    try:
        result = subprocess.run(arguments, capture_output=True, text=True, timeout=15)
    except (OSError, subprocess.TimeoutExpired):
        return None
    return result.stdout.strip() if result.returncode == 0 else None


def tool_directory_candidates(sdk):
    custom = os.environ.get("UNREAL4D_METAL_TOOLCHAIN")
    if custom:
        root = Path(custom).expanduser()
        yield root
        yield root / "usr/bin"
        yield root / ("macos" if sdk == "macosx" else "ios") / "bin"
        return

    compiler = command_output(["xcrun", "--sdk", sdk, "--find", "metal"])
    if compiler:
        yield Path(compiler).parent

    developer = os.environ.get("DEVELOPER_DIR") or command_output(["xcode-select", "-p"])
    if developer:
        for toolchain in sorted((Path(developer) / "Toolchains").glob("*.xctoolchain")):
            yield toolchain / "usr/bin"

    # Recent Xcode versions install Metal as a separately mounted component.
    # These are system locations; no user name, volume, or Xcode version is fixed.
    locations = (
        "/var/run/com.apple.security.cryptexd/mnt/com.apple.MobileAsset.MetalToolchain-*/Metal.xctoolchain/usr/bin",
        "/Library/Developer/Toolchains/*.xctoolchain/usr/bin",
        str(Path.home() / "Library/Developer/Toolchains/*.xctoolchain/usr/bin"),
    )
    for location in locations:
        for directory in sorted(glob.glob(location), reverse=True):
            yield Path(directory)


def find_tool_directory(sdk):
    visited = set()
    for candidate in tool_directory_candidates(sdk):
        # Resolve the directory, preserving each tool's executable/symlink name.
        directory = candidate.resolve()
        if directory in visited:
            continue
        visited.add(directory)
        if not all(os.access(directory / tool, os.X_OK) for tool in METAL_TOOLS):
            continue
        if command_output([str(directory / "metal"), "--version"]):
            return directory
    raise ValueError(
        "No complete, runnable Metal toolchain found for " + sdk + ". "
        "Install Xcode's Metal Toolchain component (xcodebuild -downloadComponent MetalToolchain). "
        "A nonstandard installation can be selected with UNREAL4D_METAL_TOOLCHAIN."
    )


def update_symlink(destination, source):
    if destination.is_symlink() and os.readlink(destination) == str(source):
        return
    descriptor, temporary = tempfile.mkstemp(prefix=".tool-", dir=destination.parent)
    os.close(descriptor)
    os.unlink(temporary)
    try:
        os.symlink(source, temporary)
        os.replace(temporary, destination)
    finally:
        if os.path.lexists(temporary):
            os.unlink(temporary)


def metal_root(plugin_root):
    directories = {
        "macos": find_tool_directory("macosx"),
        "ios": find_tool_directory("iphoneos"),
    }
    # Keep the override path stable when the downloaded component is remounted.
    # The symlinks are refreshed on each invocation, including IDE prelaunch tasks.
    developer = os.environ.get("DEVELOPER_DIR") or command_output(["xcode-select", "-p"]) or ""
    fingerprint = hashlib.sha256(json.dumps({
        "developer": str(Path(developer).resolve()) if developer else "",
        "custom": os.environ.get("UNREAL4D_METAL_TOOLCHAIN", ""),
    }, sort_keys=True).encode()).hexdigest()[:16]
    root = plugin_root.resolve() / "BuildArtifacts/MacToolchains" / fingerprint
    for sdk, directory in directories.items():
        bin_directory = root / sdk / "bin"
        bin_directory.mkdir(parents=True, exist_ok=True)
        for tool in METAL_TOOLS:
            update_symlink(bin_directory / tool, directory / tool)
    return root


def configure_vscode(workspace, plugin_root, engine_root):
    if os.environ.get("UNREAL4D_MAC_ENVIRONMENT") != "1":
        return
    tools = Path(plugin_root).resolve() / "Tools"
    runner = str(tools / "Setup-macOS.sh")
    for task in workspace.get("tasks", {}).get("tasks", []):
        command = task.get("command")
        if command == "/bin/sh" and task.get("args", [])[:1] == [runner]:
            continue
        if task.get("type") == "shell" and command:
            task["command"] = "/bin/sh"
            task["args"] = [runner, engine_root, command] + task.get("args", [])
    for configuration in workspace.get("launch", {}).get("configurations", []):
        if configuration.get("type") == "lldb":
            environment = configuration.setdefault("env", {})
            environment["UE-CmdLineArgs"] = os.environ["UE-CmdLineArgs"]
        elif configuration.get("type") == "cppdbg":
            environment = configuration.setdefault("environment", [])
            environment[:] = [item for item in environment if item.get("name") != "UE-CmdLineArgs"]
            environment.append({"name": "UE-CmdLineArgs", "value": os.environ["UE-CmdLineArgs"]})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("ubt-args")
    metal = commands.add_parser("metal-root")
    metal.add_argument("--plugin-root", type=Path, required=True)
    editor = commands.add_parser("editor-args")
    editor.add_argument("--metal-root", type=Path, required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "ubt-args":
            print(ubt_args(os.environ.get("UBT_EXTRA_ARGS", "")))
        elif arguments.command == "metal-root":
            print(metal_root(arguments.plugin_root))
        else:
            print(editor_args(os.environ.get("UE-CmdLineArgs", ""), arguments.metal_root))
    except (OSError, ValueError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
