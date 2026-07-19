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
