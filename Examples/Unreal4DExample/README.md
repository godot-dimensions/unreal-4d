# Unreal 4D example

This is a small Unreal project for developing and validating the Unreal4D plugin. It may be used as a playground, or just as a test to see if the example assets load.

The project finds the repository's plugin through the relative `AdditionalPluginDirectories` entry in `Unreal4DExample.uproject`. It does not copy or symlink the plugin.

## Compile on Windows

Follow the instructions in the [root README](../../README.md) to compile everything. Then, in this folder:

1. Run one of the build scripts, such as `Build-Windows.bat` or `Build-Windows.ps1`.
    - This generates a Visual Studio solution (`Unreal4DExample.sln`) and VS Code configuration files, then builds the example.
2. Open the generated solution in Visual Studio.
3. Set "Unreal4DExample" as the startup project.
4. Build and run the project to start the Unreal editor.

The Windows `.bat` and `.ps1` scripts do the same thing. The `.bat` script calls the `.ps1` script, for convenience of double-clicking from Windows Explorer. Otherwise, the `.ps1` script should be ran directly, such as from a terminal, or from a build script.

The Windows build scripts support the following command-line arguments:

- `-EngineRoot` if the Unreal source code is located elsewhere.
- `-Target` set to `Editor` or `Game`
- `-Configuration` set to `Development`, `DebugGame`, or `Shipping`
- `-SkipProjectFiles` to skip regenerating project files and just recompile existing C++ files.
  - This option will not be sufficient if you change which C++ files exist on disk, such as creating new files.
- `-GenerateOnly` to regenerate Visual Studio and VS Code metadata without compiling.

When project files are generated, the script also creates a machine-specific, gitignored `.vscode` configuration at the repository root. This makes C++, Unreal reflection-aware headers, and C# module rules discoverable when the `unreal-4d` folder is opened directly in VS Code. See the [root README](../../README.md#develop-with-visual-studio-code) for setup details.
