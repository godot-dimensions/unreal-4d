# Unreal4D plugin

`Unreal4D` is the reusable plugin portion of the Unreal 4D project. It is deliberately kept separate from example and game-specific code.

## Compile on Windows

Follow the instructions in the [root README](../../README.md) to compile everything. Then, in this folder:

1. Run one of the build scripts, such as `Build-Windows.bat` or `Build-Windows.ps1`.
    - This generates build artifacts in `Plugins/Unreal4D/BuildArtifacts` which can be used in Unreal projects as a plugin dependency.

The Windows `.bat` and `.ps1` scripts do the same thing. The `.bat` script calls the `.ps1` script, for convenience of double-clicking from Windows Explorer. Otherwise, the `.ps1` script should be ran directly, such as from a terminal, or from a build script.

The Windows build scripts support the following command-line arguments:

- `-EngineRoot` if the Unreal source code is located elsewhere.
- `-PackageDir` to specify the destination for the generated plugin package. Default is `BuildArtifacts/Unreal4D-Win64`.
- `-TargetPlatforms` passed to Unreal's BuildPlugin. Default is `Win64`.

## Compile on macOS or Linux

Run `Build-macOS.command` on macOS or `Build-Linux.sh` on Linux. Both are lightweight wrappers around the shared `Build-Unix.sh` implementation.

On macOS, the shared `Tools/MacEnvironment.sh` setup supplies the same Apple Clang warning exceptions used by the example and Quantum Hosky. It uses UnrealBuildTool's `UBT_EXTRA_ARGS` environment hook, including builds started by `RunUAT BuildPlugin`. It also discovers the installed Metal tools and supplies an override inherited by shader workers. Missing tools are reported before packaging starts. The generated override is stored in ignored `BuildArtifacts/MacToolchains`; no engine source modifications or machine-specific source paths are required.

`Tools/Setup-macOS.sh` is an internal wrapper for generated IDE tasks. It accepts an engine root and a command, sets up that command's macOS environment, and executes it. Use `Build-macOS.command` to package the plugin from Finder or a terminal.

The Unix build scripts support the following command-line arguments:

- `--engine-root` if the Unreal source code is located elsewhere.
- `--package-dir` to specify the destination for the generated plugin package.
- `--target-platforms` passed to Unreal's BuildPlugin. The default is automatically selected as `Mac` or `Linux`.

The build scripts only package the plugin. They do not update this repository or the Unreal Engine checkout.

## Folder struccture

This folder contains the plugin, and within it:

- `Shaders/` contains plugin shader files.
- `Source/` contains the plugin source code (the bulk of the repository).
  - `Unreal4DCore/` contains runtime-facing 4D types and systems.
  - `Unreal4DRendering/` owns renderer integration and plugin shaders.
  - `Unreal4DEditor/` contains editor-only tools and integration.

Project and gameplay modules should normally depend on `Unreal4DCore`. Core may use Rendering internally; consumers should only add a direct Rendering dependency when they include its public interfaces.

### Shaders

Plugin shader files belong in `Shaders/`. `Unreal4DRendering` maps this directory to the virtual shader root:

```text
/Plugin/Unreal4D
```

For example, `Shaders/Private/Example.usf` would be referenced as `/Plugin/Unreal4D/Private/Example.usf`.
