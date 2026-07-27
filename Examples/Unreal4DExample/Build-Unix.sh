#!/bin/sh

# Generates VS Code metadata and builds the Unreal4D example on macOS or Linux.

set -eu

engine_root=
target=Editor
configuration=Development
skip_project_files=0
generate_only=0

usage() {
    printf 'Usage: %s [--engine-root PATH] [--target Editor|Game] [--configuration Development|DebugGame|Shipping] [--skip-project-files] [--generate-only]\n' "$0"
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

case "$target" in
    Editor|editor) target=Editor ;;
    Game|game) target=Game ;;
    *) die "--target must be Editor or Game." ;;
esac

case "$configuration" in
    Development|DebugGame|Shipping) ;;
    *) die "--configuration must be Development, DebugGame, or Shipping." ;;
esac

if [ "$skip_project_files" -eq 1 ] && [ "$generate_only" -eq 1 ]; then
    die "--skip-project-files and --generate-only cannot be used together."
fi

case "$(uname -s)" in
    Darwin)
        host_platform=Mac
        engine_python_relative=Engine/Binaries/ThirdParty/Python3/Mac/bin/python3
        build_tool_relative=Engine/Build/BatchFiles/Mac/Build.sh
        ;;
    Linux)
        host_platform=Linux
        engine_python_relative=Engine/Binaries/ThirdParty/Python3/Linux/bin/python3
        build_tool_relative=Engine/Build/BatchFiles/Linux/Build.sh
        ;;
    *)
        die "This script supports macOS and Linux only."
        ;;
esac

script_parent=$(dirname "$0")
script_dir=$(CDPATH= cd -P "$script_parent" && pwd)
repository_root=$(CDPATH= cd -P "$script_dir/../.." && pwd)

if [ -z "$engine_root" ]; then
    engine_root=${UNREAL_ENGINE_ROOT:-}
fi
if [ -z "$engine_root" ]; then
    engine_root=$script_dir/../../../UnrealEngine5
fi
[ -d "$engine_root" ] ||
    die "Unreal Engine was not found at '$engine_root'. Pass --engine-root or set UNREAL_ENGINE_ROOT."
engine_root=$(CDPATH= cd -P "$engine_root" && pwd)

project_file=$script_dir/Unreal4DExample.uproject
generate_project_files=$engine_root/GenerateProjectFiles.sh
build_tool=$engine_root/$build_tool_relative

[ -f "$project_file" ] || die "Project descriptor not found: $project_file"

if [ "$skip_project_files" -eq 0 ]; then
    [ -f "$generate_project_files" ] ||
        die "GenerateProjectFiles.sh was not found beneath '$engine_root'."

    if command -v python3 >/dev/null 2>&1; then
        python_command=$(command -v python3)
    elif [ -x "$engine_root/$engine_python_relative" ]; then
        python_command=$engine_root/$engine_python_relative
    else
        die "Python 3 was not found in PATH or beneath '$engine_root'."
    fi

    "$generate_project_files" \
        "-Project=$project_file" \
        -Game \
        -Engine \
        -CurrentPlatform \
        -Automated \
        -VSCode

    "$python_command" - "$repository_root" "$script_dir" <<'PYTHON'
import json
import os
import sys

repository_root = os.path.realpath(sys.argv[1])
example_directory = os.path.realpath(sys.argv[2])
generated_vscode_directory = os.path.join(example_directory, ".vscode")
generated_cpp_configuration = os.path.join(generated_vscode_directory, "c_cpp_properties.json")
generated_workspace = os.path.join(example_directory, "Unreal4DExample.code-workspace")


def read_json(path):
    if not os.path.isfile(path):
        raise SystemExit(f"UnrealBuildTool did not generate '{path}'.")
    try:
        with open(path, "r", encoding="utf-8-sig") as file:
            return json.load(file)
    except (OSError, json.JSONDecodeError) as error:
        raise SystemExit(f"Could not parse '{path}'.\n{error}")


def write_json(path, value):
    temporary_path = path + ".tmp"
    with open(temporary_path, "w", encoding="utf-8", newline="\n") as file:
        json.dump(value, file, indent=4)
        file.write("\n")
    os.replace(temporary_path, path)


cpp_configuration = read_json(generated_cpp_configuration)
workspace = read_json(generated_workspace)

compile_commands = None
for configuration in cpp_configuration.get("configurations", []):
    candidates = configuration.get("compileCommands", [])
    if isinstance(candidates, str):
        candidates = [candidates]
    for candidate in candidates:
        candidate_path = os.path.expanduser(candidate)
        if not os.path.isabs(candidate_path):
            candidate_path = os.path.join(generated_vscode_directory, candidate_path)
        candidate_path = os.path.realpath(candidate_path)
        if os.path.isfile(candidate_path):
            compile_commands = candidate_path
            break
    if compile_commands:
        break

if compile_commands is None:
    raise SystemExit(
        f"UnrealBuildTool did not provide an existing compile database in '{generated_cpp_configuration}'."
    )

plugin_workspace_path = "../../Plugins/Unreal4D"
folders = workspace.get("folders", [])
if not isinstance(folders, list):
    folders = []
folders = [
    folder
    for folder in folders
    if folder.get("name") != "Unreal4D" and folder.get("path") != plugin_workspace_path
]
plugin_folder = {"name": "Unreal4D", "path": plugin_workspace_path}
workspace["folders"] = folders[:1] + [plugin_folder] + folders[1:]

settings = workspace.setdefault("settings", {})
if not isinstance(settings, dict):
    settings = {}
    workspace["settings"] = settings
settings["C_Cpp.default.compileCommands"] = compile_commands

solution_path = settings.get("dotnet.defaultSolution")
if solution_path:
    expanded_solution_path = os.path.expanduser(solution_path)
    if not os.path.isabs(expanded_solution_path):
        expanded_solution_path = os.path.join(example_directory, expanded_solution_path)
    if not os.path.isfile(expanded_solution_path):
        settings.pop("dotnet.defaultSolution", None)

extensions = workspace.setdefault("extensions", {})
if not isinstance(extensions, dict):
    extensions = {}
    workspace["extensions"] = extensions
recommendations = extensions.get("recommendations", [])
if not isinstance(recommendations, list):
    recommendations = []
recommendations.extend(
    [
        "ms-vscode.cpptools",
        "ms-dotnettools.csharp",
        "ms-dotnettools.csdevkit",
    ]
)
extensions["recommendations"] = sorted(set(recommendations))

write_json(generated_workspace, workspace)

repository_vscode_directory = os.path.join(repository_root, ".vscode")
os.makedirs(repository_vscode_directory, exist_ok=True)

repository_cpp_configuration = json.loads(json.dumps(cpp_configuration))
for configuration in repository_cpp_configuration.get("configurations", []):
    candidates = configuration.get("compileCommands", [])
    candidates_were_a_string = isinstance(candidates, str)
    if candidates_were_a_string:
        candidates = [candidates]
    normalized_candidates = []
    for candidate in candidates:
        candidate_path = os.path.expanduser(candidate)
        if not os.path.isabs(candidate_path):
            candidate_path = os.path.join(generated_vscode_directory, candidate_path)
        normalized_candidates.append(os.path.realpath(candidate_path))
    if candidates_were_a_string and normalized_candidates:
        configuration["compileCommands"] = normalized_candidates[0]
    else:
        configuration["compileCommands"] = normalized_candidates

write_json(
    os.path.join(repository_vscode_directory, "c_cpp_properties.json"),
    repository_cpp_configuration,
)

repository_settings = dict(settings)
if "dotnet.defaultSolution" in repository_settings:
    solution_name = os.path.basename(repository_settings["dotnet.defaultSolution"])
    repository_settings["dotnet.defaultSolution"] = (
        f"Examples/Unreal4DExample/{solution_name}"
    )

write_json(
    os.path.join(repository_vscode_directory, "settings.json"),
    repository_settings,
)
write_json(
    os.path.join(repository_vscode_directory, "extensions.json"),
    extensions,
)
write_json(
    os.path.join(repository_vscode_directory, "tasks.json"),
    workspace.get("tasks", {"version": "2.0.0", "tasks": []}),
)
write_json(
    os.path.join(repository_vscode_directory, "launch.json"),
    workspace.get("launch", {"version": "0.2.0", "configurations": []}),
)

print(f"Added the Unreal4D plugin root and IntelliSense settings to '{generated_workspace}'.")
print(f"Generated repository-local VS Code configuration in '{repository_vscode_directory}'.")
PYTHON
fi

if [ "$generate_only" -eq 1 ]; then
    printf 'Successfully generated VS Code project metadata.\n'
    exit 0
fi

[ -f "$build_tool" ] || die "Build.sh was not found beneath '$engine_root'."

if [ "$target" = Editor ]; then
    target_name=Unreal4DExampleEditor
else
    target_name=Unreal4DExample
fi

"$build_tool" \
    "$target_name" \
    "$host_platform" \
    "$configuration" \
    "-Project=$project_file" \
    -WaitMutex

printf 'Successfully built %s for %s %s.\n' "$target_name" "$host_platform" "$configuration"
